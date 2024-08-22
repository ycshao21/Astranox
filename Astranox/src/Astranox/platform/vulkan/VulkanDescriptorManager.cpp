#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanDescriptorManager.hpp"

#include "Astranox/platform/vulkan/VulkanUniformBufferArray.hpp"
#include "Astranox/platform/vulkan/VulkanUniformBuffer.hpp"
#include "Astranox/platform/vulkan/VulkanShader.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

#include "Astranox/rendering/Renderer.hpp"

namespace Astranox
{
    VulkanDescriptorManager::VulkanDescriptorManager()
    {
        // Descriptor pool >>>
        std::vector<VkDescriptorPoolSize> poolSizes{
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },            // Uniform buffer
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },    // Uniform buffer dynamic
        };

        //uint32_t framesInFlight = Renderer::getConfig().framesInFlight;
        VkDescriptorPoolCreateInfo poolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = 1000,
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data()
        };

        VkDevice device = VulkanContext::get()->getDevice()->getRaw();
        VK_CHECK(::vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool));
        // <<< Descriptor pool
    }

    VulkanDescriptorManager::~VulkanDescriptorManager()
    {
        VkDevice device = VulkanContext::get()->getDevice()->getRaw();
        ::vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
    }

    void VulkanDescriptorManager::createDescriptorSets(Ref<Shader> shader, Ref<UniformBufferArray> uba)
    {
        auto device = VulkanContext::get()->getDevice();
        uint32_t framesInFlight = Renderer::getConfig().framesInFlight;

        auto layout = shader.as<VulkanShader>()->getDescriptorSetLayout(0);

        m_DescriptorSets.clear();
        m_DescriptorSets.resize(framesInFlight);

        for (uint32_t frameIndex = 0; frameIndex < framesInFlight; ++frameIndex)
        {
            // Descriptor set >>>
            VkDescriptorSetAllocateInfo allocInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = m_DescriptorPool,
                .descriptorSetCount = 1,
                .pSetLayouts = &layout
            };

            VkDescriptorSet& descriptorSet = m_DescriptorSets[frameIndex].emplace_back();
            VK_CHECK(::vkAllocateDescriptorSets(device->getRaw(), &allocInfo, &descriptorSet));
            // <<< Descriptor set

            // Descriptor writes >>>
            auto& bufferInfo = uba->getBuffer(frameIndex).as<VulkanUniformBuffer>()->getDescriptorBufferInfo();

            std::vector<VkWriteDescriptorSet> writeDescriptors{
                // Uniform buffer
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = descriptorSet,
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .pImageInfo = nullptr,
                    .pBufferInfo = &bufferInfo,
                    .pTexelBufferView = nullptr
                }
            };

            ::vkUpdateDescriptorSets(
                device->getRaw(),
                static_cast<uint32_t>(writeDescriptors.size()),
                writeDescriptors.data(),
                0,
                nullptr);
        }
    }
}
