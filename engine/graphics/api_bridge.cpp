#include "api_bridge.h"
#include "api_interface.h"
#include "vulkan/api_vulkan.h"
#include "window.h"

namespace BE_NAMESPACE
{
auto APIBridge::initialize(Window& window, bool enable_validation_layers, E_API desired_api) -> bool
{
    switch (desired_api)
    {
        case E_API::API_VULKAN:
            m_current_api = std::make_unique<APIVulkan>(window, enable_validation_layers);
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

void APIBridge::draw_frame() { m_current_api->draw_frame(); }
}  // namespace BE_NAMESPACE