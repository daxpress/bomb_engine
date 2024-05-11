add_target_subdirectory(bomb_engine_graphics PRIVATE)

target_sources(bomb_engine_graphics
	PUBLIC 
	PRIVATE 
	"api_vulkan.h" "api_vulkan.cpp" "api_vulkan_structs.h" "api_vulkan_structs.cpp" 
)