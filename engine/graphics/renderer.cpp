#include "renderer.h"
#include "api_bridge.h"
#include "window.h"

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
	void Renderer::draw_frame()
	{
		m_api_bridge->draw_frame();
	}
}