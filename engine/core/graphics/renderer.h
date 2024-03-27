#pragma once

#include "core/bomb_engine.h"
#include "core/graphics/window.h"

#include <vulkan/vulkan.hpp>

namespace core::graphics
{
	class BOMB_ENGINE_API Render
	{
	public:
		Render(const Window& window, bool enable_validation_layers);
		~Render();

	private:
		const std::vector<const char*> m_validation_layers{
			"VK_LAYER_KHRONOS_validation"
		};

		vk::UniqueInstance m_vulkan_instance;

	private:

		void create_instance(const Window& window, bool enable_validation_layers);
	};
}