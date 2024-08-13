#include "pch.hpp"
#include "Astranox/rendering/IndexBuffer.hpp"
#include "Astranox/rendering/RendererAPI.hpp"

#include "Astranox/platform/vulkan/VulkanIndexBuffer.hpp"

namespace Astranox
{
    Ref<IndexBuffer> IndexBuffer::create(uint32_t* data, uint32_t bytes)
    {
        switch (RendererAPI::getType())
        {
            case RendererAPI::Type::None:  { AST_CORE_ASSERT(false, "RendererAPI::None is not supported!"); break; }
            case RendererAPI::Type::Vulkan: { return Ref<VulkanIndexBuffer>::create(data, bytes); }
        }

        AST_CORE_ASSERT(false, "Unknown Renderer API!");
        return nullptr;
    }
}