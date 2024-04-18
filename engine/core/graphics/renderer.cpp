#include "core/graphics/renderer.h"
#include "core/graphics/api_bridge.h"

namespace bomb_engine
{
	Renderer::Renderer(Window& window, bool enable_validation_layers)
	{
		m_api_bridge = std::make_unique<APIBridge>();
		bool result = m_api_bridge->initialize(window, enable_validation_layers, E_API::API_VULKAN);
	}

	Renderer::~Renderer()
	{

	}
}