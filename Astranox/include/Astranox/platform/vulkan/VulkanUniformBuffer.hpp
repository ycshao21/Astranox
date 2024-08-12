#pragma once
#include "Astranox/rendering/UniformBuffer.hpp"
#include "Astranox/platform/vulkan/VulkanDevice.hpp"

namespace Astranox
{
    class VulkanUniformBuffer: public UniformBuffer
    {
    public:
        VulkanUniformBuffer(uint32_t bytes);
        virtual ~VulkanUniformBuffer();

        virtual void setData(const void* data) override;

        const VkDescriptorBufferInfo& getDescriptorBufferInfo() const { return m_DescriptorBufferInfo; }

    private:
        Ref<VulkanDevice> m_Device = nullptr;

        uint32_t m_Bytes = 0;

        VkBuffer m_UniformBuffer = VK_NULL_HANDLE;
        VkDeviceMemory m_UniformBufferMemory = VK_NULL_HANDLE;
        void* m_MappedUniformBuffer = nullptr;

        VkDescriptorBufferInfo m_DescriptorBufferInfo{};
    };
}

