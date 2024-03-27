# macro to add the specified directory to the engine's target directories
# module_name is there only to help finding out what CMakeLists we are looking at (for now)
macro(add_engine_module module_name)
	file (RELATIVE_PATH relative ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
	target_include_directories(bomb_engine_core PUBLIC
		${relative}
	)
endmacro()

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

target_link_libraries(bomb_engine_core glfw glm::glm)
