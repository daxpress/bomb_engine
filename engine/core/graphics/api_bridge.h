#pragma once

#include "core/bomb_engine.h"
#include "core/graphics/window.h"
#include "core/graphics/apis.h"

#include <memory>

namespace core::graphics
{
	namespace api
	{
		class IAPI;
	}

	class BOMB_ENGINE_API APIBridge
	{
	public:
		APIBridge() = default;

		[[nodiscard]]
		bool initialize(const Window& window, bool enable_validation_layers, E_API desired_api);

		~APIBridge();

	private:
		// not using unique_ptr because it doesn't allow forward declaration of interface (or at least I couldn't find a solution)
		api::IAPI* m_current_api = nullptr;
	};
}