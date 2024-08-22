#pragma once

#include "Astranox/rendering/IndexBuffer.hpp"
#include "VulkanDevice.hpp"

namespace Astranox
{
    class VulkanIndexBuffer: public IndexBuffer
    {
    public:
        VulkanIndexBuffer(uint32_t* data, uint32_t bytes);
        virtual ~VulkanIndexBuffer();

        VkBuffer getRaw() { return m_IndexBuffer; }

        virtual uint32_t getCount() const override { return m_Count; }

    private:
        Ref<VulkanDevice> m_Device = nullptr;

        uint32_t m_Count = 0;

        VkBuffer m_IndexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory m_IndexBufferMemory = VK_NULL_HANDLE;
    };
}
