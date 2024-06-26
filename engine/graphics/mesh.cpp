#include "mesh.h"

#include "vertex_data.h"

namespace BE_NAMESPACE
{
Mesh::Mesh(const std::string& file_path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file_path.c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    // tinyobj triangulates the vertices by default, so they are unique
    std::unordered_map<VertexData, uint32_t> unique_vertices{};

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            VertexData vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.tex_coord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                // 1 - y is to flip the model on y axis from here
            };

            //vertex.color = {
            //    attrib.colors[index.vertex_index + 0],
            //    attrib.colors[index.vertex_index + 1],
            //    attrib.colors[index.vertex_index + 2]
            //};

            vertex.color = { 1.0f, 1.0f, 1.0f };

            if (unique_vertices.count(vertex) == 0)
            {
                unique_vertices[vertex] = static_cast<uint32_t>(m_vertices.size());
                m_vertices.push_back(vertex);
            }
            m_indices.push_back(unique_vertices[vertex]);
        }
    }
}
}  // namespace BE_NAMESPACE