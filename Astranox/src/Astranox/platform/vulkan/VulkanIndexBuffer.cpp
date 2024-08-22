#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanIndexBuffer.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanMemoryAllocator.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

namespace Astranox
{
    VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* data, uint32_t bytes)
        : m_Count(bytes / sizeof(uint32_t))
    {
        m_Device = VulkanContext::get()->getDevice();

        VulkanMemoryAllocator allocator("VulkanIndexBuffer");

        VkBufferCreateInfo stagingBufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = bytes,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAllocation = allocator.createBuffer(
            stagingBufferCI,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            stagingBuffer
        );

        // Upload data to staging buffer
        void* dest = allocator.mapMemory<void>(stagingBufferAllocation);
        std::memcpy(dest, data, bytes);
        allocator.unmapMemory(stagingBufferAllocation);

        VkBufferCreateInfo indexBufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = bytes,
            .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        m_IndexBufferAllocation = allocator.createBuffer(
            indexBufferCI,
            VMA_MEMORY_USAGE_GPU_ONLY,
            m_IndexBuffer
        );
        
        allocator.copyBuffer(stagingBuffer, m_IndexBuffer, bytes);

        allocator.destroyBuffer(stagingBuffer, stagingBufferAllocation);
    }

    VulkanIndexBuffer::~VulkanIndexBuffer()
    {
        VulkanMemoryAllocator allocator("VulkanIndexBuffer");
        allocator.destroyBuffer(m_IndexBuffer, m_IndexBufferAllocation);
    }
}
