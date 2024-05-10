#pragma once

#include "core/bomb_engine.h"
#include "core/graphics/window.h"
#include "core/graphics/api_bridge.h"

namespace bomb_engine
{
	class BOMB_ENGINE_API Renderer
	{
	public:
		Renderer(Window& window, bool enable_validation_layers);
		~Renderer();

		void draw_frame();

	private:
		std::unique_ptr<APIBridge> m_api_bridge;
	};
}