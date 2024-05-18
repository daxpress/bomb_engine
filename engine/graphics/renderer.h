#pragma once

#include "window.h"
#include "api_bridge.h"

namespace BE_NAMESPACE
{
	class Renderer
	{
	public:
		Renderer(Window& window, bool enable_validation_layers);
		~Renderer();

		void draw_frame();

	private:
		std::unique_ptr<APIBridge> m_api_bridge;
	};
}