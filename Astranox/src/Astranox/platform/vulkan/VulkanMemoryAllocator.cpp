#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanMemoryAllocator.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

namespace Astranox
{
    VulkanMemoryAllocator::VulkanMemoryAllocator(const std::string& debugName)
        : m_debugName(debugName)
    {
    }

    void VulkanMemoryAllocator::init(Ref<VulkanDevice> device)
    {
        VmaAllocatorCreateInfo allocatorInfo{
            .flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
            .physicalDevice = device->getPhysicalDevice()->getRaw(),
            .device = device->getRaw(),
            .instance = VulkanContext::get()->getInstance(),
            .vulkanApiVersion = VK_API_VERSION_1_3,
        };

        VK_CHECK(::vmaCreateAllocator(&allocatorInfo, &s_allocator));
    }

    void VulkanMemoryAllocator::shutdown()
    {
        ::vmaDestroyAllocator(s_allocator);
    }

    VmaAllocation VulkanMemoryAllocator::createBuffer(
        VkBufferCreateInfo& bufferCreateInfo,
        VmaMemoryUsage memoryUsage,
        VkBuffer& buffer)
    {
        VmaAllocationCreateInfo allocationCreateInfo{
            .usage = memoryUsage,
        };

        VmaAllocation allocation;
        VK_CHECK(::vmaCreateBuffer(s_allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr));
        AST_CORE_DEBUG("VulkanMemoryAllocator \"{0}\": Allocated buffer of {1} bytes", m_debugName, bufferCreateInfo.size);

        return allocation;
    }

    VmaAllocation VulkanMemoryAllocator::createImage(VkImageCreateInfo& imageCreateInfo, VmaMemoryUsage memoryUsage, VkImage& image)
    {
        VmaAllocationCreateInfo allocationCreateInfo{
            .usage = memoryUsage,
        };

        VmaAllocation allocation;
        VK_CHECK(::vmaCreateImage(s_allocator, &imageCreateInfo, &allocationCreateInfo, &image, &allocation, nullptr));

        VmaAllocationInfo allocationInfo;
        ::vmaGetAllocationInfo(s_allocator, allocation, &allocationInfo);
        AST_CORE_DEBUG("VulkanMemoryAllocator \"{0}\": Allocated image of {1}x{2} pixels, {3} bytes", m_debugName, imageCreateInfo.extent.width, imageCreateInfo.extent.height, allocationInfo.size);

        return allocation;
    }

    void VulkanMemoryAllocator::destroyBuffer(VkBuffer& buffer, VmaAllocation allocation)
    {
        AST_CORE_DEBUG("VulkanMemoryAllocator \"{0}\": Destroyed buffer", m_debugName);
        ::vmaDestroyBuffer(s_allocator, buffer, allocation);
    }

    void VulkanMemoryAllocator::destroyImage(VkImage& image, VmaAllocation allocation)
    {
        AST_CORE_DEBUG("VulkanMemoryAllocator \"{0}\": Destroyed image", m_debugName);
        ::vmaDestroyImage(s_allocator, image, allocation);
    }

    void VulkanMemoryAllocator::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize bytes)
    {
        auto device = VulkanContext::get()->getDevice();
        auto commandPool = device->getCommandPool();

        VkCommandBuffer commandBuffer = commandPool->allocateCommandBuffer();
        commandPool->beginOneTimeBuffer(commandBuffer);

        VkBufferCopy copyRegion{
            .srcOffset = 0,
            .dstOffset = 0,
            .size = bytes
        };
        ::vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        commandPool->endOneTimeBuffer(commandBuffer);
    }

    void VulkanMemoryAllocator::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        auto device = VulkanContext::get()->getDevice();
        auto commandPool = device->getCommandPool();

        VkCommandBuffer commandBuffer = commandPool->allocateCommandBuffer();
        commandPool->beginOneTimeBuffer(commandBuffer);

        VkBufferImageCopy region{
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            },
            .imageOffset = { 0, 0, 0 },
            .imageExtent = { width, height, 1 }
        };

        ::vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        commandPool->endOneTimeBuffer(commandBuffer);
    }

    void VulkanMemoryAllocator::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
    {
        auto device = VulkanContext::get()->getDevice();
        auto commandPool = device->getCommandPool();

        VkCommandBuffer commandBuffer = commandPool->allocateCommandBuffer();
        commandPool->beginOneTimeBuffer(commandBuffer);

        VkImageMemoryBarrier barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = 0,
            .oldLayout = oldLayout,
            .newLayout = newLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = mipLevels,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            AST_CORE_ASSERT(false, "Unsupported layout transition");
        }

        ::vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        commandPool->endOneTimeBuffer(commandBuffer);
    }
}
