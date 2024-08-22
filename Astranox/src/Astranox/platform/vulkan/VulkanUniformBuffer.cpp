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
        VulkanMemoryAllocator allocator("VulkanUniformBuffer");

        VkBufferCreateInfo uniformBufferCI{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = bytes,
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        m_UniformBufferAllocation = allocator.createBuffer(
            uniformBufferCI,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            m_UniformBuffer
        );

        m_MappedUniformBuffer = allocator.mapMemory<void>(m_UniformBufferAllocation);

        m_DescriptorBufferInfo = {
            .buffer = m_UniformBuffer,
            .offset = 0,
            .range = bytes
        };
    }

    VulkanUniformBuffer::~VulkanUniformBuffer()
    {
        VulkanMemoryAllocator allocator("VulkanUniformBuffer");
        allocator.unmapMemory(m_UniformBufferAllocation);
        allocator.destroyBuffer(m_UniformBuffer, m_UniformBufferAllocation);
    }

    void VulkanUniformBuffer::setData(const void* data, uint32_t bytes, uint32_t offset)
    {
        std::memcpy(m_MappedUniformBuffer, (const uint8_t*)data + offset, bytes);
    }
}