#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace BE_NAMESPACE
{
struct VertexData
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 tex_coord;

    static auto get_binding_description() -> vk::VertexInputBindingDescription;
    static auto get_attribute_descriptions() -> std::vector<vk::VertexInputAttributeDescription>;

    inline auto operator==(const VertexData& other) const -> bool
    {
        return pos == other.pos && color == other.color && tex_coord == other.tex_coord;
    }
};
}  // namespace BE_NAMESPACE

// required to allow comparisons and use in hashsets and maps

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std
{
    template<> struct hash<BE_NAMESPACE::VertexData>
    {
        // some hash magic with bit manipulation
        size_t operator()(BE_NAMESPACE::VertexData const& vertex) const
        {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.tex_coord) << 1);
        }
    };
}