#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanMemoryAllocator.hpp"
#include "Astranox/platform/vulkan/VulkanUniformBuffer.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"
#include "Astranox/platform/vulkan/VulkanUtils.hpp"

namespace Astranox
{
    VulkanUniformBuffer::VulkanUniformBuffer(uint32_t bytes)
        : m_Bytes(bytes)
    {
        m_Device = VulkanContext::get()->getDevice();

        VulkanMemoryAllocator::createBuffer(
            bytes,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_UniformBuffer,
            m_UniformBufferMemory
        );

        VK_CHECK(::vkMapMemory(m_Device->getRaw(), m_UniformBufferMemory, 0, bytes, 0, &m_MappedUniformBuffer));

        m_DescriptorBufferInfo = {
            .buffer = m_UniformBuffer,
            .offset = 0,
            .range = bytes
        };
    }

    VulkanUniformBuffer::~VulkanUniformBuffer()
    {
        ::vkUnmapMemory(m_Device->getRaw(), m_UniformBufferMemory);

        VulkanMemoryAllocator::destroyBuffer(m_UniformBuffer, m_UniformBufferMemory);
    }

    void VulkanUniformBuffer::setData(const void* data, uint32_t bytes, uint32_t offset)
    {
        std::memcpy(m_MappedUniformBuffer, (const uint8_t*)data + offset, bytes);
    }
}