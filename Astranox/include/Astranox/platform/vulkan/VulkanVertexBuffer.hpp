#pragma once

#include "Astranox/rendering/VertexBuffer.hpp"
#include "VulkanDevice.hpp"

namespace Astranox
{
    class VulkanVertexBuffer: public VertexBuffer
    {
    public:
        VulkanVertexBuffer(void* data, uint32_t bytes);
        virtual ~VulkanVertexBuffer();

        virtual void bind() override;

        VkBuffer getRaw() { return m_VertexBuffer; }

    private:
        Ref<VulkanDevice> m_Device = nullptr;

        VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory m_VertexBufferMemory = VK_NULL_HANDLE;
    };
}
