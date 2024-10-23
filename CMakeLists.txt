# CMakeList.txt : CMake project for bomb_engine, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.28)

# Enable Hot Reload for MSVC compilers if supported.
#if (POLICY CMP0141)
#  cmake_policy(SET CMP0141 NEW)
#  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
#endif()

# setup clang tidy
set(clang_tidy_checks "modernize-*,clang-analyzer-*,performance-*,portability-*,google-*,concurrency-*,-google-readability-todo")
set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=${clang_tidy_checks} -header-filter=.*)

project (bomb_engine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_SCAN_FOR_MODULES Off)
set(CMAKE_EXPORT_COMPILE_COMMANDS On)


include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
  set(CMAKE_BUILD_PARALLEL_LEVEL${N})
endif()

# precompiled headers stuff
set(CMAKE_PCH_INSTANTIATE_TEMPLATES on)
# I wonder why did I even bother using precompiled headers in the first place... Waiting for good c++ modules support at this point

include(helpers.cmake)

# ====================================== GLOBAL VARIABLES =====================================

set_property(GLOBAL PROPERTY header_tool_targets)
set_property(GLOBAL PROPERTY header_tool_products_dir)

# =================================== ENGINE SUBDIRECTORIES ===================================

add_subdirectory(engine)
add_subdirectory(editor)
add_subdirectory(plugins)