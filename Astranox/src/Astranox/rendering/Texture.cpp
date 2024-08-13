#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanTexture.hpp"
#include "Astranox/rendering/RendererAPI.hpp"

namespace Astranox
{
    Ref<Texture> Texture::create(const std::filesystem::path& path, bool enableMipMaps)
    {
        switch (RendererAPI::getType())
        {
            case RendererAPI::Type::None:  { AST_CORE_ASSERT(false, "RendererAPI::None is not supported!"); break; }
            case RendererAPI::Type::Vulkan: { return Ref<VulkanTexture>::create(path, enableMipMaps); }
        }

        AST_CORE_ASSERT(false, "Unknown Renderer API!");
        return nullptr;
    }
}
