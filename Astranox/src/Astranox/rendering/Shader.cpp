#include "pch.hpp"
#include "Astranox/rendering/Shader.hpp"
#include "Astranox/platform/vulkan/VulkanShader.hpp"

namespace Astranox
{
    Ref<Shader> Shader::create(const std::string& vertexPath, const std::string& fragmentPath)
    {
        return Ref<VulkanShader>::create(vertexPath, fragmentPath);
    }
}

