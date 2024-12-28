#include <vertex_data.h>

namespace BE_NAMESPACE
{
class Mesh
{
public:
    explicit Mesh(const std::string& file_path);

    std::vector<uint32_t> m_indices;
    std::vector<VertexData> m_vertices;
};
}  // namespace BE_NAMESPACE