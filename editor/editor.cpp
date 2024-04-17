// editor.cpp : Defines the entry point for the application.
//

#include "core/graphics/window.h"
#include "core/graphics/renderer.h"

#ifdef _DEBUG
const bool enable_layers = true;
#else
const bool enable_layers = false;
#endif

int main()
{
	bomb_engine::Window window{1920, 1080, "Bomb Engine Editor"};
	auto renderer = bomb_engine::Render{ window, enable_layers };

	while(window.is_open()) 
	{
		window.poll_events();
	}

	return 0;
}
