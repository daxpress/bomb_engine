#pragma once

#include "api_bridge.h"

namespace bomb_engine
{
	class IAPI
	{
	public:
		virtual ~IAPI() {};
		
		inline virtual E_API get_api() = 0;

		virtual void draw_frame() {};
	};
}