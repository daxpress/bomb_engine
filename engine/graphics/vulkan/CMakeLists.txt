add_target_subdirectory(bomb_engine_graphics PRIVATE)

target_sources(bomb_engine_graphics
PRIVATE
	"api_vulkan.cpp" "api_vulkan_structs.cpp"  
PRIVATE
FILE_SET HEADERS FILES
	"api_vulkan.h" "api_vulkan_structs.h" 
 
)