#include "pch.hpp"
#include "Astranox/rendering/UniformBuffer.hpp"

#include "Astranox/platform/vulkan/VulkanUniformBuffer.hpp"

namespace Astranox
{
    Ref<UniformBuffer> UniformBuffer::create(uint32_t bytes)
    {
        return Ref<VulkanUniformBuffer>::create(bytes);
    }
}