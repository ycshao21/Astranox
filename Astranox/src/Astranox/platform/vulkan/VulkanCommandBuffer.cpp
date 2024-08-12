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
        VK_CHECK(::vkCreateCommandPool(device->getRaw(), &poolInfo, nullptr, &m_GraphicsCommandPool));
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
            .commandPool = m_GraphicsCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VK_CHECK(::vkAllocateCommandBuffers(device->getRaw(), &allocInfo, &commandBuffer));

        return commandBuffer;
    }

    std::vector<VkCommandBuffer> VulkanCommandPool::allocateCommandBuffers(uint32_t count)
    {
        auto device = VulkanContext::get()->getDevice();

        VkCommandBufferAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_GraphicsCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = count,
        };

        std::vector<VkCommandBuffer> commandBuffers(count);
        VK_CHECK(::vkAllocateCommandBuffers(device->getRaw(), &allocInfo, commandBuffers.data()));

        return commandBuffers;
    }

    void VulkanCommandPool::beginOneTimeBuffer(VkCommandBuffer commandBuffer)
    {
        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };
        VK_CHECK(::vkBeginCommandBuffer(commandBuffer, &beginInfo));
    }

    void VulkanCommandPool::endOneTimeBuffer(VkCommandBuffer commandBuffer)
    {
        auto device = VulkanContext::get()->getDevice();

        VK_CHECK(::vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer,
        };
        VK_CHECK(::vkQueueSubmit(device->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE));
        ::vkQueueWaitIdle(device->getGraphicsQueue());

        ::vkFreeCommandBuffers(device->getRaw(), m_GraphicsCommandPool, 1, &commandBuffer);
    }
}

