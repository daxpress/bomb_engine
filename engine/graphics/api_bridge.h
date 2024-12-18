#pragma once

#include "window.h"
#include "e_api.h"
#include "api_interface.h"

namespace BE_NAMESPACE
{
class [[expose]] APIBridge
{
public:
    APIBridge() = default;

    [[nodiscard]]
    auto initialize(Window& window, bool enable_validation_layers, E_API desired_api) -> bool;

    // a placeholder for testing while working on the vulkan renderer, later we'll fix
    // responsibilities
    void draw_frame();

    ~APIBridge() = default;

private:
    std::unique_ptr<IAPI> m_current_api = nullptr;
};
}  // namespace BE_NAMESPACE