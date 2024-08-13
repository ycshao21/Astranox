#include "pch.hpp"
#include "Astranox/rendering/UniformBuffer.hpp"
#include "Astranox/rendering/RendererAPI.hpp"

#include "Astranox/platform/vulkan/VulkanUniformBuffer.hpp"

namespace Astranox
{
    Ref<UniformBuffer> UniformBuffer::create(uint32_t bytes)
    {
        switch (RendererAPI::getType())
        {
            case RendererAPI::Type::None:  { AST_CORE_ASSERT(false, "RendererAPI::None is not supported!"); break; }
            case RendererAPI::Type::Vulkan: { return Ref<VulkanUniformBuffer>::create(bytes); }
        }

        AST_CORE_ASSERT(false, "Unknown Renderer API!");
        return nullptr;
    }
}