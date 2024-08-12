#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "Astranox/rendering/VertexBuffer.hpp"
#include "Astranox/rendering/IndexBuffer.hpp"
#include <filesystem>

namespace Astranox
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texCoord;

        bool operator==(const Vertex& other) const
        {
            return position == other.position
                && color == other.color
                && texCoord == other.texCoord;
        }
    };

    using Index = uint32_t;


    class Mesh
    {
    public:
        Mesh() = default;
        Mesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices)
            : m_Vertices(vertices), m_Indices(indices)
        {
            m_VertexBuffer = VertexBuffer::create(m_Vertices.data(), sizeof(Vertex) * static_cast<uint32_t>(m_Vertices.size()));
            m_IndexBuffer = IndexBuffer::create(m_Indices.data(), sizeof(Index) * static_cast<uint32_t>(m_Indices.size()));
        }
        virtual ~Mesh()
        {
            m_VertexBuffer = nullptr;
            m_IndexBuffer = nullptr;
        }

    public:
        const std::vector<Vertex>& getVertices() const { return m_Vertices; }
        const std::vector<Index>& getIndices() const { return m_Indices; }

        Ref<VertexBuffer> getVertexBuffer() { return m_VertexBuffer; }
        Ref<IndexBuffer> getIndexBuffer() { return m_IndexBuffer; }

    private:
        std::vector<Vertex> m_Vertices;
        std::vector<Index> m_Indices;

        Ref<VertexBuffer> m_VertexBuffer = nullptr;
        Ref<IndexBuffer> m_IndexBuffer = nullptr;
    };


    Mesh readMesh(const std::filesystem::path& path);
}
