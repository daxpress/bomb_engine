#pragma once

#include "bomb_engine.h"

namespace BE_NAMESPACE
{
	struct BOMB_ENGINE_API VertexData
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 tex_coord;

		static vk::VertexInputBindingDescription get_binding_description();
		static std::vector<vk::VertexInputAttributeDescription> get_attribute_descriptions();
	};
}