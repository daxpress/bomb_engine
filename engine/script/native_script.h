#pragma once

#include "scriptable.h"

namespace BE_NAMESPACE
{
	class NativeScript
	{
	public:
		void start();
		void update(float tick);
	private:
		int _placeholder;
		// so basically entt components must not be empty (which makes sense, howver in this case I don't really have data yet)
	};
}