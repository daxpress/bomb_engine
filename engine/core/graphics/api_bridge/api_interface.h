#pragma once

#include "core/graphics/api_bridge.h"

namespace core::graphics::api
{
	class IAPI
	{
	public:
		virtual ~IAPI() {};
		
		inline virtual E_API get_api() = 0;
	};
}