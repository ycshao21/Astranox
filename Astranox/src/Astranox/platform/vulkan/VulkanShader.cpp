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

    std::vector<char> VulkanShader::readShaderFile(const std::string& codeFile)
    {
        std::ifstream file(codeFile, std::ios::ate | std::ios::binary);
        AST_CORE_ASSERT(file.is_open(), "Failed to open file: {0}", codeFile);

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> code(fileSize);

        file.seekg(0);
        file.read(code.data(), fileSize);

        file.close();

        return code;
    }

    VulkanShader::VulkanShader(const std::string& vertexPath, const std::string& fragmentPath)
    {
        auto vertexCode = readShaderFile(vertexPath);
        auto fragmentCode = readShaderFile(fragmentPath);

        createShaders(vertexCode, fragmentCode);
    }

    VulkanShader::~VulkanShader()
    {
        destroy();
    }

    void VulkanShader::destroy()
    {
        auto device = VulkanContext::get()->getDevice();

        ::vkDestroyShaderModule(device->getRaw(), m_VertexShaderModule, nullptr);
        ::vkDestroyShaderModule(device->getRaw(), m_FragmentShaderModule, nullptr);
    }

    void VulkanShader::createShaders(const std::vector<char>& vertexCode, const std::vector<char>& fragmentCode)
    {
        auto device = VulkanContext::get()->getDevice();

        VkShaderModuleCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        };

        createInfo.codeSize = vertexCode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(vertexCode.data());
        VkResult result = ::vkCreateShaderModule(device->getRaw(), &createInfo, nullptr, &m_VertexShaderModule);
        VK_CHECK(result);

        VkPipelineShaderStageCreateInfo vertexShaderStage{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = m_VertexShaderModule,
            .pName = "main",
            .pSpecializationInfo = nullptr,
        };
        m_ShaderStages.push_back(vertexShaderStage);

        createInfo.codeSize = fragmentCode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentCode.data());
        result = ::vkCreateShaderModule(device->getRaw(), &createInfo, nullptr, &m_FragmentShaderModule);
        VK_CHECK(result);

        VkPipelineShaderStageCreateInfo fragmentShaderStage{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = m_FragmentShaderModule,
            .pName = "main",
            .pSpecializationInfo = nullptr,
        };
        m_ShaderStages.push_back(fragmentShaderStage);
    }
}
