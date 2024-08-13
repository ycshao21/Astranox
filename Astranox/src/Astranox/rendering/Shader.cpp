#include "pch.hpp"
#include "Astranox/rendering/Shader.hpp"
#include "Astranox/rendering/RendererAPI.hpp"
#include "Astranox/platform/vulkan/VulkanShader.hpp"

namespace Astranox
{
    Ref<Shader> Shader::create(const std::string& name, const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath)
    {
        switch (RendererAPI::getType())
        {
            case RendererAPI::Type::None:  { AST_CORE_ASSERT(false, "RendererAPI::None is not supported!"); break; }
            case RendererAPI::Type::Vulkan: { return Ref<VulkanShader>::create(name, vertexPath, fragmentPath); }
        }

        AST_CORE_ASSERT(false, "Unknown Renderer API!");
        return nullptr;
    }



    void ShaderLibrary::add(Ref<Shader> shader)
    {
        auto& name = shader->getName();
        add(name, shader);
    }

    void ShaderLibrary::add(const std::string& name, Ref<Shader> shader)
    {
        AST_CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end(), "Shader already exists!");
        m_Shaders[name] = shader;
    }

    Ref<Shader> ShaderLibrary::load(const std::string& name, const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath)
    {
        auto shader = Shader::create(name, vertexPath, fragmentPath);
        add(shader);
        return shader;
    }

    Ref<Shader> ShaderLibrary::get(const std::string& name)
    {
        AST_CORE_ASSERT(m_Shaders.find(name) != m_Shaders.end(), "Shader not found!");
        return m_Shaders[name];
    }
}

