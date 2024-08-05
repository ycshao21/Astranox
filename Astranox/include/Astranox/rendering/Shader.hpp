#pragma once

#include <string>

namespace Astranox
{
    class Shader: public RefCounted
    {
    public:
        static Ref<Shader> create(const std::string& vertexPath, const std::string& fragmentPath);
        virtual ~Shader() = default;

        virtual void createShaders(const std::vector<char>& vertexCode, const std::vector<char>& fragmentCode) = 0;

        virtual void bind() = 0;
        virtual void unbind() = 0;
    };
}
