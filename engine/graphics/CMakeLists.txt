new_engine_module(bomb_engine_graphics OBJECT)

target_sources(bomb_engine_graphics
PRIVATE
	"renderer.h" "window.h" "api_bridge.h" "graphics_pipeline_type.h" "api_interface.h" 
	"spirv_shader.h" "vertex_data.h"
	"renderer.cpp" "window.cpp" "api_bridge.cpp"  "spirv_shader.cpp" "vertex_data.cpp"
)

add_subdirectory(vulkan)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(spirv-cross REQUIRED)

target_link_libraries(bomb_engine_graphics
bomb_engine_utilities
glfw glm::glm Vulkan::Vulkan spirv-cross::spirv-cross)