add_engine_module(api_bridge PRIVATE)

target_sources(bomb_engine_core
	PUBLIC 
	PRIVATE "api_interface.h" "api_vulkan.h" "api_vulkan.cpp"
)