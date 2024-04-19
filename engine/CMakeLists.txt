project(bomb_engine_core)

add_library(bomb_engine_core SHARED)

target_include_directories(bomb_engine_core 
	PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(core)

# add external libraries here! it helps with keeping track of all the third party stuff
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(spirv-cross REQUIRED)

target_link_libraries(bomb_engine_core glfw glm::glm Vulkan::Vulkan spirv-cross::spirv-cross)
