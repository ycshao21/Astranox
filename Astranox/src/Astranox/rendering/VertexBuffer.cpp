#include "pch.hpp"
#include "Astranox/rendering/VertexBuffer.hpp"

#include "Astranox/platform/vulkan/VulkanVertexBuffer.hpp"

namespace Astranox
{
    Ref<VertexBuffer> VertexBuffer::create(void* data, uint32_t bytes)
    {
        return Ref<VulkanVertexBuffer>::create(data, bytes);
    }
}