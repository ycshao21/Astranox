#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanIndexBuffer.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanBufferManager.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

namespace Astranox
{
    VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* data, uint32_t bytes)
        : m_Count(bytes / sizeof(uint32_t))
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
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_IndexBuffer,
            m_IndexBufferMemory
        );
        VulkanBufferManager::copyBuffer(
            stagingBuffer,
            m_IndexBuffer,
            bytes
        );
        VulkanBufferManager::destroyBuffer(
            stagingBuffer,
            stagingBufferMemory
        );
    }

    VulkanIndexBuffer::~VulkanIndexBuffer()
    {
        VulkanBufferManager::destroyBuffer(
            m_IndexBuffer,
            m_IndexBufferMemory
        );
    }

    void VulkanIndexBuffer::bind()
    {
        //vkCmdBindIndexBuffer(getCurrentCommandBuffer(), m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
    }
}
