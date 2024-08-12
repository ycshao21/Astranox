#pragma once
#include <vector>
#include <string>

#include "Astranox/core/Base.hpp"

namespace Astranox
{
    enum class ShaderDataType: uint8_t
    {
        None = 0,
        Float, Vec2, Vec3, Vec4,
        Mat3, Mat4,
        Int, Ivec2, Ivec3, Ivec4,
        Bool
    };

    constexpr uint32_t calculateShaderDataTypeSize(ShaderDataType type)
    {
        switch (type)
        {
            case ShaderDataType::Float: { return 4; }
            case ShaderDataType::Vec2:  { return 4 * 2; }
            case ShaderDataType::Vec3:  { return 4 * 3; }
            case ShaderDataType::Vec4:  { return 4 * 4; }
            case ShaderDataType::Mat3:  { return 4 * 3 * 3; }
            case ShaderDataType::Mat4:  { return 4 * 4 * 4; }
            case ShaderDataType::Int:   { return 4; }
            case ShaderDataType::Ivec2: { return 4 * 2; }
            case ShaderDataType::Ivec3: { return 4 * 3; }
            case ShaderDataType::Ivec4: { return 4 * 4; }
            case ShaderDataType::Bool:  { return 1; }
        }

        AST_CORE_ASSERT(false, "Unknown shader data type!");
        return 0;
    }

    struct VertexBufferElement
    {
        std::string name;
        ShaderDataType dataType;
        uint32_t size;
        uint32_t offset;
        bool normalized;

        VertexBufferElement(ShaderDataType dataType_, const std::string& name_, bool normalized_ = false)
            : name(name_), dataType(dataType_), size(calculateShaderDataTypeSize(dataType_)), offset(0), normalized(normalized_)
        {
        }

        constexpr uint32_t getVertexCount() const
        {
            switch (dataType)
            {
                case ShaderDataType::Float: { return 1; }
                case ShaderDataType::Vec2:  { return 2; }
                case ShaderDataType::Vec3:  { return 3; }
                case ShaderDataType::Vec4:  { return 4; }
                case ShaderDataType::Mat3:  { return 3 * 3; }
                case ShaderDataType::Mat4:  { return 4 * 4; }
                case ShaderDataType::Int:   { return 1; }
                case ShaderDataType::Ivec2: { return 2; }
                case ShaderDataType::Ivec3: { return 3; }
                case ShaderDataType::Ivec4: { return 4; }
                case ShaderDataType::Bool:  { return 1; }
            }

            AST_CORE_ASSERT(false, "Unknown shader data type!");
            return 0;
        }
    };

    class VertexBufferLayout
    {
    public:
        VertexBufferLayout() = default;

        VertexBufferLayout(const std::initializer_list<VertexBufferElement>& elements)
            : m_Elements(elements)
        {
            calculateOffsetsAndStride();
        }

        const std::vector<VertexBufferElement>& getElements() const { return m_Elements; }
        uint32_t getStride() const { return m_Stride; }

    public: // Range-based for loop support
        std::vector<VertexBufferElement>::iterator begin() { return m_Elements.begin(); }
        std::vector<VertexBufferElement>::iterator end() { return m_Elements.end(); }

    private:
        void calculateOffsetsAndStride()
        {
            uint32_t offset = 0;
            m_Stride = 0;
            for (auto& element : m_Elements)
            {
                element.offset = offset;
                offset += element.size;
                m_Stride += element.size;
            }
        }

    private:
        std::vector<VertexBufferElement> m_Elements;
        uint32_t m_Stride;
    };
}
