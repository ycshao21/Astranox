#pragma once

#include <vulkan/vulkan.h>

namespace Astranox
{
    class VulkanVertexBuffer
    {
    public:
        VulkanVertexBuffer();
        virtual ~VulkanVertexBuffer() = default;

    public:
        VkBuffer getRaw() const { return m_VertexBuffer; }

    private:
        VkBuffer m_VertexBuffer = VK_NULL_HANDLE;
    };
}
