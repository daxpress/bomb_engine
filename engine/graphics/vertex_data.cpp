#include "vertex_data.h"

namespace BE_NAMESPACE
{
auto VertexData::get_binding_description() -> vk::VertexInputBindingDescription
{
    return {0, sizeof(VertexData), vk::VertexInputRate::eVertex};
}

auto VertexData::get_attribute_descriptions() -> std::vector<vk::VertexInputAttributeDescription>
{
    std::vector<vk::VertexInputAttributeDescription> attributes{};

    auto position = vk::VertexInputAttributeDescription(
        0, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, pos)
    );
    auto color = vk::VertexInputAttributeDescription(
        1, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, color)
    );
    auto tex_coord = vk::VertexInputAttributeDescription(
        2, 0, vk::Format::eR32G32Sfloat, offsetof(VertexData, tex_coord)
    );

    attributes.push_back(position);
    attributes.push_back(color);
    attributes.push_back(tex_coord);

    return attributes;
}
}  // namespace BE_NAMESPACE
