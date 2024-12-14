#include "renderer.h"

#include "api_bridge.h"
#include "window.h"

MakeCategory(Renderer);

namespace BE_NAMESPACE
{
Renderer::Renderer(Window& window, bool enable_validation_layers)
{
    m_api_bridge = std::make_unique<APIBridge>();
    auto res = m_api_bridge->initialize(window, enable_validation_layers, E_API::API_VULKAN);
    if (!res)
    {
        Log(RendererCategory, LogSeverity::Error, "Failed to initialize APIBridge");
    }
}

void Renderer::draw_frame() { m_api_bridge->draw_frame(); }
}  // namespace BE_NAMESPACE