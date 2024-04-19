add_engine_module(vulkan PRIVATE)

target_sources(bomb_engine_core
	PUBLIC 
	PRIVATE 
	"api_vulkan.h" "api_vulkan.cpp" "api_vulkan_structs.h" "api_vulkan_structs.cpp"
	"vulkan_pipeline_builder.h"  
	"vulkan_pipeline_builder.cpp" "vulkan_renderpass_builder.h" "vulkan_renderpass_builder.cpp"
)