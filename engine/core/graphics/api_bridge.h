#pragma once

#include "core/bomb_engine.h"
#include "core/graphics/window.h"

#include <memory>

namespace bomb_engine
{
	class IAPI;

	enum class E_API : uint8_t
	{
		API_VULKAN = 0,
		API_DIRECTX = 1,
	};

	class BOMB_ENGINE_API APIBridge
	{
	public:
		APIBridge() = default;

		[[nodiscard]]
		bool initialize(Window& window, bool enable_validation_layers, E_API desired_api);


		~APIBridge();

	private:
		// not using unique_ptr because it doesn't allow forward declaration of interface (or at least I couldn't find a solution)
		IAPI* m_current_api = nullptr;
	};
}