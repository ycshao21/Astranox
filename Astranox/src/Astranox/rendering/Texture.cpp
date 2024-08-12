#include "pch.hpp"
#include "Astranox/platform/vulkan/VulkanTexture.hpp"

namespace Astranox
{
    Ref<Texture> Texture::create(const std::filesystem::path& path, bool enableMipMaps)
    {
        return Ref<VulkanTexture>::create(path, enableMipMaps);
    }
}
