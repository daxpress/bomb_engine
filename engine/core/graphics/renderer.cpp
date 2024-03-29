#include "core/graphics/renderer.h"
#include "core/graphics/api_bridge.h"
#include "core/graphics/apis.h"

namespace core::graphics
{
	Render::Render(Window& window, bool enable_validation_layers)
	{
		m_api_bridge = std::make_unique<APIBridge>();
		bool result = m_api_bridge->initialize(window, enable_validation_layers, E_API::API_VULKAN);
	}

	Render::~Render()
	{

	}
}