#include "pch.hpp"
#include "Astranox/rendering/Mesh.hpp"

#include "Astranox/core/Base.hpp"
#include "tinyobjloader/tiny_obj_loader.h"

namespace std
{
    template<>
    struct hash<Astranox::Vertex>
    {
        size_t operator()(const Astranox::Vertex & vertex) const
        {
            return ((hash<glm::vec3>()(vertex.position) ^
                (hash<glm::vec4>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

namespace Astranox
{
    Mesh readMesh(const std::filesystem::path& path)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        std::string pathStr = path.string();
        bool loaded = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, pathStr.c_str());
        AST_ASSERT(loaded, "Failed to load model.");

        std::vector<Vertex> vertices;
        std::vector<Index> indices;

        std::unordered_map<Vertex, Index> uniqueVertices;
        for (auto& shape : shapes)
        {
            for (auto& index : shape.mesh.indices)
            {
                Vertex vertex{
                    .position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    },
                    .color = { 1.0f, 1.0f, 1.0f, 1.0f },
                    .texCoord = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    }
                };

                if (uniqueVertices.find(vertex) == uniqueVertices.end())
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }
        AST_INFO("Vertices: {0}, Indices: {1}", vertices.size(), indices.size());

        Mesh mesh(vertices, indices);
        return mesh;
    }
}
