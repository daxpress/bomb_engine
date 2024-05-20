project(pybomb LANGUAGES CXX)

add_library(pybomb STATIC)

target_sources(pybomb PRIVATE "pybomb.cpp")

# getting the engine headers to pass them to the script that generates binding code
set(headers)
set(libs)
get_engine_headers(headers libs)

set(pybinder "pybinder/pybinder.py")
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# runs the binder script
add_custom_command(TARGET pybomb PRE_BUILD 
	COMMAND ${Python_EXECUTABLE} ${pybinder} ARGS ${headers} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})