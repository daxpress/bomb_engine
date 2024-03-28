#include "core/graphics/api_bridge.h"
#include "core/graphics/api_bridge/api_interface.h"
#include "core/graphics/api_bridge/api_vulkan.h"

namespace core::graphics
{
	bool APIBridge::initialize(const Window& window, bool enable_validation_layers, E_API desired_api)
	{
		switch (desired_api)
		{
		case core::graphics::E_API::API_VULKAN:
			m_current_api = new api::APIVulkan(window, enable_validation_layers);
			return true;
			break;
		case core::graphics::E_API::API_DIRECTX:
			return false;
			break;
		default:
			std::unreachable();
			break;
		}
	}
	APIBridge::~APIBridge()
	{
		if (m_current_api)
		{
			delete m_current_api;
		}
	}
}