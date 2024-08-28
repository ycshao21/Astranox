#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanTexture2D.hpp"
#include "Astranox/rendering/RendererAPI.hpp"

namespace Astranox
{
    Ref<Texture2D> Texture2D::create(const std::filesystem::path& path, bool enableMipMaps)
    {
        switch (RendererAPI::getType())
        {
            case RendererAPI::Type::None:  { AST_CORE_ASSERT(false, "RendererAPI::None is not supported!"); break; }
            case RendererAPI::Type::Vulkan: { return Ref<VulkanTexture2D>::create(path, enableMipMaps); }
        }

        AST_CORE_ASSERT(false, "Unknown Renderer API!");
        return nullptr;
    }

    Ref<Texture2D> Texture2D::create(uint32_t width, uint32_t height, Buffer buffer)
    {
        switch (RendererAPI::getType())
        {
            case RendererAPI::Type::None:  { AST_CORE_ASSERT(false, "RendererAPI::None is not supported!"); break; }
            case RendererAPI::Type::Vulkan: { return Ref<VulkanTexture2D>::create(width, height, buffer); }
        }

        AST_CORE_ASSERT(false, "Unknown Renderer API!");
        return nullptr;
    }
}
