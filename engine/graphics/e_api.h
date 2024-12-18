#pragma once

namespace BE_NAMESPACE
{
    enum class [[expose]] E_API : uint8_t
    {
        API_VULKAN = 0,
        API_DIRECTX = 1,
    };
}
