#pragma once

#include "Astranox/rendering/VertexBuffer.hpp"
#include "VulkanDevice.hpp"

namespace Astranox
{
    class VulkanVertexBuffer: public VertexBuffer
    {
    public:
        VulkanVertexBuffer(uint32_t bytes);
        VulkanVertexBuffer(void* data, uint32_t bytes);
        virtual ~VulkanVertexBuffer();

        void setData(const void* data, uint32_t bytes) override;

        VkBuffer getRaw() { return m_VertexBuffer; }

    private:
        Ref<VulkanDevice> m_Device = nullptr;

        VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;
    };
}
