#include "pch.hpp"
#include "Astranox/rendering/UniformBufferArray.hpp"

#include "Astranox/platform/vulkan/VulkanUniformBufferArray.hpp"

namespace Astranox
{
    Ref<UniformBufferArray> UniformBufferArray::create(uint32_t bytes)
    {
        return Ref<VulkanUniformBufferArray>::create(bytes);
    }
}