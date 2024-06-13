# CMakeList.txt : CMake project for bomb_engine, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.28)

# Enable Hot Reload for MSVC compilers if supported.
#if (POLICY CMP0141)
#  cmake_policy(SET CMP0141 NEW)
#  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
#endif()

set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=modernize-*,clang-analyzer-*,performance-*,portability-*,llvm-*,concurrency-*,)

project (bomb_engine LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_SCAN_FOR_MODULES Off)


include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
  set(CMAKE_BUILD_PARALLEL_LEVEL${N})
endif()

# precompiled headers stuff
set(CMAKE_PCH_INSTANTIATE_TEMPLATES on)
# I wonder why did I even bother using precompiled headers in the first place... Waiting for good c++ modules support at this point

# =================================== HELPER MACROS AND FUNCTIONS ======================================

# macro to add the current directory to the engine's target directories.
# it helps with keeping things organized including the files in a directory directly from the
# local CMakeLists.txt file.
# it is especially useful when some parts are private to the engine module.
# planning to have this also include src files
macro(add_target_subdirectory target_module access_specifier)
	file (RELATIVE_PATH relative ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
	target_include_directories(${target_module}
		${access_specifier}
		${relative}
	)
endmacro()

# adds a new engine module as a linked library, to be called from owner of the module.
# most of the time target_module = bomb_engine_engine, but it might be usefull to have some 
# linking to other libraries instead
function (add_engine_submodule target_module module_folder)
	add_subdirectory(${module_folder})
	set (module ${module_folder})
	string(PREPEND module "bomb_engine_")
	target_link_libraries(${target_module} ${module})
endfunction()

# declares a new engine module, call in the module primary CMakeLists.txt file
function(new_engine_module module_name library_type)
	message(STATUS "adding engine module: ${module_name}")

	project(${module_name})

	add_library(${module_name} ${library_type})
	target_include_directories(${module_name} 
		PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
	)

	# set dependancy to header_tool to allow generated code to compile
	add_dependencies(${module_name} header_tool)

	# builds a precompiled header for each module so that we can have common stl stuff in the engine one
	# and another one in the module that contains external libraries for the module
	get_target_property(common_pch bomb_engine_engine SOURCE_DIR)
	set(pch_list )
	get_target_property(local_pch ${module_name} SOURCE_DIR)
	if(EXISTS ${local_pch}/pch.h AND NOT ${local_pch}/pch.h STREQUAL ${common_pch}/pch.h)
		message(STATUS "found ${module_name} additional precompiled header")
		list(APPEND pch_list "${local_pch}/pch.h")
	endif()
	if(NOT ${module_name} STREQUAL "bomb_engine_engine")
		target_precompile_headers(${module_name} PUBLIC "${common_pch}/pch.h")
		target_precompile_headers(${module_name} PRIVATE ${pch_list})
	else()
	message(STATUS "skipped custom precompiled header for ${module_name}")
	endif()

endfunction()

# copies files from a target src location to a dst target location
function(copy_to_bin_dir src_target dst_target src_relative_path dst_relative_path)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		TARGET $<TARGET_PROPERTY:${src_target},SOURCE_DIR>/${src_relative_path}
		TARGET $<TARGET_FILE_DIR:${dst_target}>/${dst_relative_path}
	)
endfunction()

# populates the out_headers parameter with the engine headers list and the out_libs with the engine modules
# very useful to generate bindings for a scripting language
# do not use __linked_libs or __headers as they are variables set here in the macro
macro(get_engine_headers out_headers out_libs)
	get_target_property(__linked_libs bomb_engine_engine LINK_LIBRARIES)
	list(APPEND ${out_libs} ${__linked_libs})
	foreach(lib ${__linked_libs})
		get_target_property(__headers ${lib} HEADER_SET_HEADERS)
		list(APPEND ${out_headers} ${__headers})
		endforeach()
endmacro()

# =================================== ENGINE SUBDIRECTORIES ===================================

add_subdirectory(engine)
add_subdirectory(editor)
add_subdirectory(plugins)