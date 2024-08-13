#include "pch.hpp"
#include "Astranox/rendering/GraphicsContext.hpp"
#include "Astranox/rendering/Renderer.hpp"
#include "Astranox/platform/vulkan/VulkanContext.hpp"

namespace Astranox
{
    Ref<GraphicsContext> GraphicsContext::create()
    {
        switch (RendererAPI::getType())
        {
            case RendererAPI::Type::None:  { AST_CORE_ASSERT(false, "RendererAPI::None is not supported!"); break; }
            case RendererAPI::Type::Vulkan: { return Ref<VulkanContext>::create(); }
        }

        AST_CORE_ASSERT(false, "Unknown Renderer API!");
        return nullptr;
    }
}
