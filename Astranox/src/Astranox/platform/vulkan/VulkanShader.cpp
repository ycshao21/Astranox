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

    void VulkanShader::createShaders(const std::map<VkShaderStageFlagBits, std::vector<uint32_t>>& shaderData)
    {
        auto device = VulkanContext::get()->getDevice();

        m_ShaderStages.clear();

        for (auto& [stage, data] : shaderData)
        {
            VkShaderModuleCreateInfo createInfo{
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = data.size() * sizeof(uint32_t),
                .pCode = data.data(),
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
            for (auto& [binding, uniformBuffer] : m_ShaderDescriptorSetInfo.uniformBufferInfos)
            {
                VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

                VkDescriptorSetLayoutBinding& bindingInfo = layoutBindings.emplace_back();
                bindingInfo = {
                    .binding = binding,
                    .descriptorType = descriptorType,
                    .descriptorCount = 1,
                    .stageFlags = uniformBuffer.shaderStage,
                    .pImmutableSamplers = nullptr,
                };

                VkWriteDescriptorSet& writeDescriptorSet = m_ShaderDescriptorSetInfo.writeDescriptorSets[uniformBuffer.name];
                writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet.dstBinding = binding;
                writeDescriptorSet.dstArrayElement = 0;
                writeDescriptorSet.descriptorCount = 1;
                writeDescriptorSet.descriptorType = descriptorType;
                writeDescriptorSet.pImageInfo = nullptr;
                writeDescriptorSet.pTexelBufferView = nullptr;
            }

            for (auto& [binding, imageSampler] : m_ShaderDescriptorSetInfo.imageSamplerInfos)
            {
                VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

                VkDescriptorSetLayoutBinding& bindingInfo = layoutBindings.emplace_back();
                bindingInfo = {
                    .binding = binding,
                    .descriptorType = descriptorType,
                    .descriptorCount = imageSampler.arraySize,
                    .stageFlags = imageSampler.shaderStage,
                    .pImmutableSamplers = nullptr,
                };

                VkWriteDescriptorSet& writeDescriptorSet = m_ShaderDescriptorSetInfo.writeDescriptorSets[imageSampler.name];
                writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                writeDescriptorSet.dstBinding = binding;
                writeDescriptorSet.dstArrayElement = 0;
                writeDescriptorSet.descriptorCount = imageSampler.arraySize;
                writeDescriptorSet.descriptorType = descriptorType;
                writeDescriptorSet.pImageInfo = nullptr;
                writeDescriptorSet.pTexelBufferView = nullptr;
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
