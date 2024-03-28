#pragma once

#include "core/bomb_engine.h"
#include "core/graphics/window.h"
#include "core/graphics/api_bridge.h"

namespace core::graphics
{
	class BOMB_ENGINE_API Render
	{
	public:
		Render(const Window& window, bool enable_validation_layers);
		~Render();

	private:
		std::unique_ptr<APIBridge> m_api_bridge;
	};
}