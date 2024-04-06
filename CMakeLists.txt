# CMakeList.txt : CMake project for bomb_engine, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.28)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (bomb_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED On)

# macro to add the specified directory to the engine's target directories
# module_name is there only to help finding out what CMakeLists we are looking at (for now)
macro(add_engine_module module_name access_specifier)
	file (RELATIVE_PATH relative ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
	target_include_directories(bomb_engine_core
		${access_specifier}
		${relative}
	)
endmacro()

include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
  set(CMAKE_BUILD_PARALLEL_LEVEL${N})
endif()

add_subdirectory(engine)
add_subdirectory(editor)

# TODO: Add tests and install targets if needed.
