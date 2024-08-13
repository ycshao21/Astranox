#include "pch.hpp"
#include "Astranox/rendering/VertexBuffer.hpp"
#include "Astranox/rendering/RendererAPI.hpp"

#include "Astranox/platform/vulkan/VulkanVertexBuffer.hpp"

namespace Astranox
{
    Ref<VertexBuffer> VertexBuffer::create(void* data, uint32_t bytes)
    {
        switch (RendererAPI::getType())
        {
            case RendererAPI::Type::None:  { AST_CORE_ASSERT(false, "RendererAPI::None is not supported!"); break; }
            case RendererAPI::Type::Vulkan: { return Ref<VulkanVertexBuffer>::create(data, bytes); }
        }

        AST_CORE_ASSERT(false, "Unknown Renderer API!");
        return nullptr;
    }
}