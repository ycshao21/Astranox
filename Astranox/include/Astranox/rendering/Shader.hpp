#pragma once

#include <string>
#include <filesystem>

namespace Astranox
{
    class Shader: public RefCounted
    {
    public:
        static Ref<Shader> create(const std::string& name, const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
        virtual ~Shader() = default;

        //virtual void createShaders(const std::vector<char>& vertexCode, const std::vector<char>& fragmentCode) = 0;

        virtual const std::string& getName() const = 0;

        virtual void bind() = 0;
        virtual void unbind() = 0;
    };

    class ShaderLibrary
    {
    public:
        void add(Ref<Shader> shader);
        void add(const std::string& name, Ref<Shader> shader);

        Ref<Shader> load(const std::string& name, const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);

        Ref<Shader> get(const std::string& name);

    private:
        std::unordered_map<std::string, Ref<Shader>> m_Shaders;
    };
}
