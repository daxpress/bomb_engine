// editor.cpp : Defines the entry point for the application.
// right now it acts more like a sandbox to put everything together and test the functionalities.

#include "graphics/window.h"
#include "graphics/renderer.h"
#include "scene/scene.h"
#include "scene/entity.h"
#include "script/native_script.h"

#ifdef _DEBUG
const bool enable_layers = true;
#else
const bool enable_layers = false;
#endif

auto main() -> int
{
	// the following will most definetly be part of an Application class in the future

	// this would be a setup part
	bomb_engine::Window window{1920, 1080, "Bomb Engine Editor"};
	auto renderer = bomb_engine::Renderer{ window, enable_layers };

	// sample scene 
	auto scene = bomb_engine::Scene();
	auto entity1 = scene.spawn_entity();
	entity1.add_component<bomb_engine::Scriptable>(bomb_engine::NativeScript());

	// ended setup, now start the loop
	scene.start();

	while(window.is_open()) 
	{
		// inside the loop we tick
		window.poll_events();
		scene.update(.6f);
		renderer.draw_frame();
	}

	return 0;
} 
