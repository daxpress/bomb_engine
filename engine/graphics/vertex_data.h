#pragma once

namespace BE_NAMESPACE
{
struct VertexData
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 tex_coord;

    static auto get_binding_description() -> vk::VertexInputBindingDescription;
    static auto get_attribute_descriptions() -> std::vector<vk::VertexInputAttributeDescription>;
};
}  // namespace BE_NAMESPACE