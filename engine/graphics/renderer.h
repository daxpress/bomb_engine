#pragma once

#include "api_bridge.h"
#include "window.h"

namespace BE_NAMESPACE
{
class Renderer
{
public:
    Renderer(Window& window, bool enable_validation_layers);
    ~Renderer() = default;

    void draw_frame();

private:
    std::unique_ptr<APIBridge> m_api_bridge;
};
}  // namespace BE_NAMESPACE