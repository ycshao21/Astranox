#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanVertexBuffer.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanBufferManager.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

namespace Astranox
{
    VulkanVertexBuffer::VulkanVertexBuffer(uint32_t bytes)
    {
        m_Device = VulkanContext::get()->getDevice();

        VulkanBufferManager::createBuffer(
            bytes,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_VertexBuffer,
            m_VertexBufferMemory
        );
    }

    VulkanVertexBuffer::VulkanVertexBuffer(void* data, uint32_t bytes)
    {
        m_Device = VulkanContext::get()->getDevice();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        VulkanBufferManager::createBuffer(
            bytes,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory
        );

        // Upload data to staging buffer
        void* dest = nullptr;
        VK_CHECK(::vkMapMemory(m_Device->getRaw(), stagingBufferMemory, 0, bytes, 0, &dest));
        std::memcpy(dest, data, bytes);
        ::vkUnmapMemory(m_Device->getRaw(), stagingBufferMemory);

        VulkanBufferManager::createBuffer(
            bytes,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_VertexBuffer,
            m_VertexBufferMemory
        );
        VulkanBufferManager::copyBuffer(
            stagingBuffer,
            m_VertexBuffer,
            bytes
        );
        VulkanBufferManager::destroyBuffer(
            stagingBuffer,
            stagingBufferMemory
        );
    }

    VulkanVertexBuffer::~VulkanVertexBuffer()
    {
        VulkanBufferManager::destroyBuffer(
            m_VertexBuffer,
            m_VertexBufferMemory
        );
    }

    void VulkanVertexBuffer::setData(const void* data, uint32_t bytes)
    {
        void* dest = nullptr;
        VK_CHECK(::vkMapMemory(m_Device->getRaw(), m_VertexBufferMemory, 0, bytes, 0, &dest));
        std::memcpy(dest, data, bytes);
        ::vkUnmapMemory(m_Device->getRaw(), m_VertexBufferMemory);
    }
}
