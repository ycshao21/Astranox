#include "pch.hpp"
#include "Astranox/rendering/GraphicsContext.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"

namespace Astranox
{
    Ref<GraphicsContext> GraphicsContext::create()
    {
        return Ref<VulkanContext>::create();
    }
}
