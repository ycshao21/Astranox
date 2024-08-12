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
        uint32_t framesInFlight = Renderer::getConfig().framesInFlight;

        // Descriptor pool >>>
        std::vector<VkDescriptorPoolSize> poolSizes{
            // Uniform buffer
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = framesInFlight
            },
            // Sampler
            {
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = framesInFlight
            }
        };

        VkDescriptorPoolCreateInfo poolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = framesInFlight,
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

    void VulkanDescriptorManager::init(VkDescriptorSetLayout layout, VkSampler sampler, VkImageView imageView, Ref<UniformBufferArray> uba)
    {
        auto device = VulkanContext::get()->getDevice();
        uint32_t framesInFlight = Renderer::getConfig().framesInFlight;

        // Descriptor sets >>>
        std::vector<VkDescriptorSetLayout> layouts(framesInFlight, layout);
        VkDescriptorSetAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = m_DescriptorPool,
            .descriptorSetCount = static_cast<uint32_t>(framesInFlight),
            .pSetLayouts = layouts.data()
        };

        m_DescriptorSets.resize(framesInFlight);
        VK_CHECK(::vkAllocateDescriptorSets(device->getRaw(), &allocInfo, m_DescriptorSets.data()));

        for (uint32_t i = 0; i < framesInFlight; i++)
        {
            // Sampler
            VkDescriptorImageInfo imageInfo{
                .sampler = sampler,
                .imageView = imageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };

            std::vector<VkWriteDescriptorSet> descriptorWrites{
                // Uniform buffer
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = m_DescriptorSets[i],
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    .pImageInfo = nullptr,
                    .pBufferInfo = &uba->getBuffer(i).as<VulkanUniformBuffer>()->getDescriptorBufferInfo(),
                    .pTexelBufferView = nullptr
                },
                // Sampler
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = m_DescriptorSets[i],
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .pImageInfo = &imageInfo,
                    .pBufferInfo = nullptr,
                    .pTexelBufferView = nullptr
                }
            };

            ::vkUpdateDescriptorSets(
                device->getRaw(),
                static_cast<uint32_t>(descriptorWrites.size()),
                descriptorWrites.data(),
                0,
                nullptr
            );
        }
        // <<< Descriptor sets
    }
}
