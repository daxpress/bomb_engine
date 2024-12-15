#pragma once

#include "window.h"

namespace BE_NAMESPACE
{
class IAPI;

enum class [[expose]] E_API : uint8_t
{
    API_VULKAN = 0,
    API_DIRECTX = 1,
};

class [[expose]] APIBridge
{
public:
    APIBridge() = default;

    [[nodiscard]]
    auto initialize(Window& window, bool enable_validation_layers, E_API desired_api) -> bool;

    // a placeholder for testing while working on the vulkan renderer, later we'll fix
    // responsabilities
    void draw_frame();

    ~APIBridge();

private:
    // not using unique_ptr because it doesn't allow forward declaration of interface (or at least I
    // couldn't find a solution)
    IAPI* m_current_api = nullptr;
};
}  // namespace BE_NAMESPACE