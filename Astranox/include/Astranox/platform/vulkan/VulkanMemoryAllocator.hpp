#pragma once
#include "vk_mem_alloc.h"

#include "VulkanDevice.hpp"
#include "VulkanUtils.hpp"


namespace Astranox
{
    class VulkanMemoryAllocator
    {
    public:
        VulkanMemoryAllocator(const std::string& debugName);

        static void init(Ref<VulkanDevice> device);
        static void shutdown();

        VmaAllocation createBuffer(
            VkBufferCreateInfo& bufferCreateInfo,
            VmaMemoryUsage memoryUsage,
            VkBuffer& buffer);

        VmaAllocation createImage(
            VkImageCreateInfo& imageCreateInfo,
            VmaMemoryUsage memoryUsage,
            VkImage& image);

        void destroyBuffer(VkBuffer& buffer, VmaAllocation allocation);

        void destroyImage(VkImage& image, VmaAllocation allocation);

        template<typename T>
        T* mapMemory(VmaAllocation allocation)
        {
            T* data;
            VK_CHECK(::vmaMapMemory(s_allocator, allocation, reinterpret_cast<void**>(&data)));
            return data;
        }

        void unmapMemory(VmaAllocation allocation)
        {
            ::vmaUnmapMemory(s_allocator, allocation);
        }

    public:
        void copyBuffer(
            VkBuffer srcBuffer,
            VkBuffer dstBuffer,
            VkDeviceSize bytes
        );

        void copyBufferToImage(
            VkBuffer buffer,
            VkImage image,
            uint32_t width,
            uint32_t height
        );

    public:
        void transitionImageLayout(
            VkImage image,
            VkFormat format,
            VkImageLayout oldLayout,
            VkImageLayout newLayout,
            uint32_t mipLevels
        );

    private:
        std::string m_debugName;

        inline static VmaAllocator s_allocator = nullptr;
    };
}
