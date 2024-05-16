#pragma once

#include "api_bridge.h"

namespace BE_NAMESPACE
{
	class IAPI
	{
	public:
		virtual ~IAPI() {};
		
		inline virtual E_API get_api() = 0;

		virtual void draw_frame() {};
	};
}