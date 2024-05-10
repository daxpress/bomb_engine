#include "vertex_data.h"

namespace bomb_engine
{
    vk::VertexInputBindingDescription VertexData::get_binding_description()
    {
        return vk::VertexInputBindingDescription(0, sizeof(VertexData), vk::VertexInputRate::eVertex);
    }

    std::vector<vk::VertexInputAttributeDescription> VertexData::get_attribute_descriptions()
    {
        std::vector<vk::VertexInputAttributeDescription> attributes{};

        auto position  = vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, pos));
        auto color     = vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(VertexData, color));
        auto tex_coord = vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(VertexData, tex_coord));

        attributes.push_back(std::move(position));
        attributes.push_back(std::move(color));
        attributes.push_back(std::move(tex_coord));

        return attributes;
    }
}

