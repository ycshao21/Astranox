#include "pch.hpp"
#include "Astranox/rendering/Shader.hpp"
#include "Astranox/platform/vulkan/VulkanShader.hpp"

namespace Astranox
{
    Ref<Shader> Shader::create(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath)
    {
        return Ref<VulkanShader>::create(vertexPath, fragmentPath);
    }
}

