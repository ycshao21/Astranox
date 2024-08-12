#include "pch.hpp"
#include "Astranox/rendering/IndexBuffer.hpp"

#include "Astranox/platform/vulkan/VulkanIndexBuffer.hpp"

namespace Astranox
{
    Ref<IndexBuffer> IndexBuffer::create(uint32_t* data, uint32_t bytes)
    {
        return Ref<VulkanIndexBuffer>::create(data, bytes);
    }
}