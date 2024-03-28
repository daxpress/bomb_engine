#pragma once

#include <cinttypes>

namespace core::graphics
{
	enum class E_API : uint8_t
	{
		API_VULKAN = 0,
		API_DIRECTX = 1,
	};
}