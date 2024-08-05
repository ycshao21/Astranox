#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanCommandBuffer.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

namespace Astranox
{
    VulkanCommandPool::VulkanCommandPool()
    {
        auto device = VulkanContext::get()->getDevice();
        auto& queueFamilyIndices = device->getPhysicalDevice()->getQueueIndices();

        VkCommandPoolCreateInfo poolInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(),
        };
        VkResult result = ::vkCreateCommandPool(device->getRaw(), &poolInfo, nullptr, &m_GraphicsCommandPool);
        VK_CHECK(result);
    }

    VulkanCommandPool::~VulkanCommandPool()
    {
        auto device = VulkanContext::get()->getDevice();
        ::vkDestroyCommandPool(device->getRaw(), m_GraphicsCommandPool, nullptr);
    }

    VkCommandBuffer VulkanCommandPool::allocateCommandBuffer()
    {
        auto device = VulkanContext::get()->getDevice();

        VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = m_GraphicsCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VkResult result = ::vkAllocateCommandBuffers(device->getRaw(), &allocInfo, &commandBuffer);
        VK_CHECK(result);

        return commandBuffer;
    }

    std::vector<VkCommandBuffer> VulkanCommandPool::allocateCommandBuffers(uint32_t count)
    {
        auto device = VulkanContext::get()->getDevice();

        VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = m_GraphicsCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = count,
        };

        std::vector<VkCommandBuffer> commandBuffers(count);
        VkResult result = ::vkAllocateCommandBuffers(device->getRaw(), &allocInfo, commandBuffers.data());
        VK_CHECK(result);

        return commandBuffers;
    }
}

