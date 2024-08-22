#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanShader.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

namespace Astranox
{
    void VulkanShader::bind()
    {
    }

    void VulkanShader::unbind()
    {
    }

    std::vector<char> VulkanShader::readCompiledShaderFile(const std::filesystem::path& codeFile)
    {
        std::ifstream file(codeFile, std::ios::ate | std::ios::binary);
        AST_CORE_ASSERT(file.is_open(), "Failed to open file: {0}", codeFile.string());

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> code(fileSize);

        file.seekg(0);
        file.read(code.data(), fileSize);

        file.close();

        return code;
    }

    VulkanShader::VulkanShader(const std::string& name, const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath)
        : m_Name(name)
    {
        auto vertexCode = readCompiledShaderFile(vertexPath);
        auto fragmentCode = readCompiledShaderFile(fragmentPath);

        std::map<VkShaderStageFlagBits, std::vector<char>> shaderData{
            { VK_SHADER_STAGE_VERTEX_BIT, vertexCode },
            { VK_SHADER_STAGE_FRAGMENT_BIT, fragmentCode }
        };
        createShaders(shaderData);
    }

    VulkanShader::~VulkanShader()
    {
        destroy();
    }

    void VulkanShader::destroy()
    {
        auto device = VulkanContext::get()->getDevice();

        for (auto layout : m_DescriptorSetLayouts)
        {
            ::vkDestroyDescriptorSetLayout(device->getRaw(), layout, nullptr);
        }

        for (auto stage : m_ShaderStages)
        {
            ::vkDestroyShaderModule(device->getRaw(), stage.module, nullptr);
        }
    }

    void VulkanShader::createShaders(const std::map<VkShaderStageFlagBits, std::vector<char>>& shaderData)
    {
        auto device = VulkanContext::get()->getDevice();

        m_ShaderStages.clear();

        for (auto& [stage, code] : shaderData)
        {
            VkShaderModuleCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = code.size(),
                .pCode = reinterpret_cast<const uint32_t*>(code.data()),
            };

            VkShaderModule shaderModule;
            VK_CHECK(::vkCreateShaderModule(device->getRaw(), &createInfo, nullptr, &shaderModule));

            VkPipelineShaderStageCreateInfo shaderStage{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = stage,
                .module = shaderModule,
                .pName = "main",
                .pSpecializationInfo = nullptr,
            };
            m_ShaderStages.push_back(shaderStage);
        }
    }

    void VulkanShader::createDescriptorSetLayouts()
    {
        uint32_t setCount = 1;
        m_DescriptorSetLayouts.clear();
        m_DescriptorSetLayouts.resize(setCount);

        for (uint32_t setIndex = 0; setIndex < setCount; ++setIndex)
        {
            std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
            for (auto& [binding, info] : m_UniformBufferInfos)
            {
                VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

                VkDescriptorSetLayoutBinding& bindingInfo = layoutBindings.emplace_back();
                bindingInfo = {
                    .binding = binding,
                    .descriptorType = descriptorType,
                    .descriptorCount = 1,
                    .stageFlags = info.shaderStage,
                    .pImmutableSamplers = nullptr,
                };
            }

            VkDescriptorSetLayoutCreateInfo layoutInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
                .pBindings = layoutBindings.data(),
            };

            VK_CHECK(::vkCreateDescriptorSetLayout(
                VulkanContext::get()->getDevice()->getRaw(),
                &layoutInfo,
                nullptr,
                &m_DescriptorSetLayouts[setIndex]));
        }
    }
}
