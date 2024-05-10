#include "core/graphics/api_bridge.h"
#include "core/graphics/api_interface.h"
#include "core/graphics/vulkan/api_vulkan.h"

namespace bomb_engine
{
	bool APIBridge::initialize(Window& window, bool enable_validation_layers, E_API desired_api)
	{
		switch (desired_api)
		{
		case E_API::API_VULKAN:
			m_current_api = new APIVulkan(window, enable_validation_layers);
			return true;
			break;
		case E_API::API_DIRECTX:
			return false;
			break;
		default:
			std::unreachable();
			break;
		}
	}

	void APIBridge::draw_frame()
	{
		m_current_api->draw_frame();
	}

	APIBridge::~APIBridge()
	{
		if (m_current_api)
		{
			delete m_current_api;
		}
	}
}