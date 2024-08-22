#pragma once
#include "Astranox/rendering/UniformBuffer.hpp"
#include "Astranox/platform/vulkan/VulkanDevice.hpp"
#include "vk_mem_alloc.h"

namespace Astranox
{
    class VulkanUniformBuffer: public UniformBuffer
    {
    public:
        VulkanUniformBuffer(uint32_t bytes);
        virtual ~VulkanUniformBuffer();

        virtual void setData(const void* data, uint32_t bytes, uint32_t offset) override;

        const VkDescriptorBufferInfo& getDescriptorBufferInfo() const { return m_DescriptorBufferInfo; }

    private:
        Ref<VulkanDevice> m_Device = nullptr;

        uint32_t m_Bytes = 0;

        VkBuffer m_UniformBuffer = VK_NULL_HANDLE;
        VmaAllocation m_UniformBufferAllocation;
        void* m_MappedUniformBuffer = nullptr;

        VkDescriptorBufferInfo m_DescriptorBufferInfo{};
    };
}

