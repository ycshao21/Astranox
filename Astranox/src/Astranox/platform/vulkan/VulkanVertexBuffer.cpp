#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanVertexBuffer.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanMemoryAllocator.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

namespace Astranox
{
    VulkanVertexBuffer::VulkanVertexBuffer(uint32_t bytes)
    {
        m_Device = VulkanContext::get()->getDevice();

        VulkanMemoryAllocator allocator("VulkanVertexBuffer");

        VkBufferCreateInfo vertexBufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = bytes,
            .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        m_VertexBufferAllocation = allocator.createBuffer(
            vertexBufferCI,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            m_VertexBuffer
        );
    }

    VulkanVertexBuffer::VulkanVertexBuffer(void* data, uint32_t bytes)
    {
        m_Device = VulkanContext::get()->getDevice();

        VulkanMemoryAllocator allocator("VulkanVertexBuffer");

        VkBufferCreateInfo stagingBufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = bytes,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferMemory = allocator.createBuffer(
            stagingBufferCI,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            stagingBuffer
        );

        // Upload data to staging buffer
        void* dest = allocator.mapMemory<void>(stagingBufferMemory);
        std::memcpy(dest, data, bytes);
        allocator.unmapMemory(stagingBufferMemory);

        VkBufferCreateInfo vertexBufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = bytes,
            .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        m_VertexBufferAllocation = allocator.createBuffer(
            vertexBufferCI,
            VMA_MEMORY_USAGE_GPU_ONLY,
            m_VertexBuffer
        );

        allocator.copyBuffer(stagingBuffer, m_VertexBuffer, bytes);

        allocator.destroyBuffer(stagingBuffer, stagingBufferMemory);
    }

    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
        VulkanMemoryAllocator allocator("VulkanVertexBuffer");
        allocator.destroyBuffer(m_VertexBuffer, m_VertexBufferAllocation);
    }

    void VulkanVertexBuffer::setData(const void* data, uint32_t bytes)
    {
        VulkanMemoryAllocator allocator("VulkanVertexBuffer");
        void* dest = allocator.mapMemory<void>(m_VertexBufferAllocation);
        std::memcpy(dest, data, bytes);
        allocator.unmapMemory(m_VertexBufferAllocation);
    }
}
