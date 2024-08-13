#include "pch.hpp"
#include "Astranox/rendering/UniformBufferArray.hpp"
#include "Astranox/rendering/RendererAPI.hpp"

#include "Astranox/platform/vulkan/VulkanUniformBufferArray.hpp"

namespace Astranox
{
    Ref<UniformBufferArray> UniformBufferArray::create(uint32_t bytes)
    {
        switch (RendererAPI::getType())
        {
            case RendererAPI::Type::None:  { AST_CORE_ASSERT(false, "RendererAPI::None is not supported!"); break; }
            case RendererAPI::Type::Vulkan: { return Ref<VulkanUniformBufferArray>::create(bytes); }
        }

        AST_CORE_ASSERT(false, "Unknown Renderer API!");
        return nullptr;
    }
}