#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanBufferManager.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

namespace Astranox
{

    void VulkanBufferManager::createBuffer(
        VkDeviceSize bufferSize,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory
    )
    {
        auto device = VulkanContext::get()->getDevice();

        VkBufferCreateInfo bufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = bufferSize,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        VK_CHECK(::vkCreateBuffer(device->getRaw(), &bufferCreateInfo, nullptr, &buffer));

        VkMemoryRequirements memoryRequirements;
        ::vkGetBufferMemoryRequirements(device->getRaw(), buffer, &memoryRequirements);

        auto& memoryProperties = device->getPhysicalDevice()->getMemoryProperties();
        uint32_t memoryTypeIndex = 0;
        for (; memoryTypeIndex < memoryProperties.memoryTypeCount; memoryTypeIndex++)
        {
            if ((memoryRequirements.memoryTypeBits & BIT(memoryTypeIndex))
                && memoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & properties)
            {
                break;
            }
        }
        AST_CORE_ASSERT(memoryTypeIndex < memoryProperties.memoryTypeCount, "Failed to find suitable memory type");

        VkMemoryAllocateInfo allocateInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = memoryTypeIndex,
        };
        VK_CHECK(::vkAllocateMemory(device->getRaw(), &allocateInfo, nullptr, &bufferMemory));

        VK_CHECK(::vkBindBufferMemory(device->getRaw(), buffer, bufferMemory, 0));
    }

    void VulkanBufferManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
    {
        auto device = VulkanContext::get()->getDevice();
        auto commandPool = device->getCommandPool();

        VkCommandBuffer commandBuffer = commandPool->allocateCommandBuffer();
        commandPool->beginOneTimeBuffer(commandBuffer);

        VkBufferCopy copyRegion{
            .srcOffset = 0,
            .dstOffset = 0,
            .size = size
        };
        ::vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        commandPool->endOneTimeBuffer(commandBuffer);
    }

    void VulkanBufferManager::destroyBuffer(VkBuffer buffer, VkDeviceMemory bufferMemory)
    {
        VkDevice device = VulkanContext::get()->getDevice()->getRaw();

        ::vkDestroyBuffer(device, buffer, nullptr);
        ::vkFreeMemory(device, bufferMemory, nullptr);
    }

    void VulkanBufferManager::createImage(
        uint32_t width,
        uint32_t height,
        uint32_t mipLevels,
        VkSampleCountFlagBits numSamples,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage& image,
        VkDeviceMemory& imageMemory
    )
    {
        auto device = VulkanContext::get()->getDevice();

        VkImageCreateInfo imageInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent = {
                .width = width,
                .height = height,
                .depth = 1
            },
            .mipLevels = mipLevels,
            .arrayLayers = 1,
            .samples = numSamples,
            .tiling = tiling,
            .usage = usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        VK_CHECK(::vkCreateImage(device->getRaw(), &imageInfo, nullptr, &image));

        VkMemoryRequirements memoryRequirements;
        ::vkGetImageMemoryRequirements(device->getRaw(), image, &memoryRequirements);

        auto& memoryProperties = device->getPhysicalDevice()->getMemoryProperties();
        uint32_t memoryTypeIndex = 0;
        for (; memoryTypeIndex < memoryProperties.memoryTypeCount; memoryTypeIndex++)
        {
            if ((memoryRequirements.memoryTypeBits & BIT(memoryTypeIndex))
                && memoryProperties.memoryTypes[memoryTypeIndex].propertyFlags & properties)
            {
                break;
            }
        }
        AST_CORE_ASSERT(memoryTypeIndex < memoryProperties.memoryTypeCount, "Failed to find suitable memory type");

        VkMemoryAllocateInfo allocateInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memoryRequirements.size,
            .memoryTypeIndex = memoryTypeIndex
        };
        VK_CHECK(::vkAllocateMemory(device->getRaw(), &allocateInfo, nullptr, &imageMemory));

        VK_CHECK(::vkBindImageMemory(device->getRaw(), image, imageMemory, 0));
    }

    void VulkanBufferManager::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
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

    void VulkanBufferManager::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
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
