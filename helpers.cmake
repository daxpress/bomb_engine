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
# most of the time target_module = bomb_engine_engine, but it might be useful to have some
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
    #
    #	# link to tools
    target_link_libraries(${module_name} bomb_engine_tools)

    # set dependency to header_tool to allow generated code to compile
    add_dependencies(${module_name} header_tool)

    set_property(GLOBAL APPEND PROPERTY header_tool_targets ${module_name})

    setup_static_analysis(${module_name})

    # needed almost everywhere...
    find_package(EnTT REQUIRED)
    target_link_libraries(${module_name} EnTT::EnTT)
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

function(new_engine_plugin plugin_name library_type)
    # it essentially does the same thing as the new_engine_module function, but it doesn't
    # require to be added to the engine with add_engine_submodule as a plugin is, by definition, optional.
    # It also does not declare a custom precompiled header by default.
    message(STATUS "adding plugin: ${plugin_name}")

    project(${plugin_name})

    add_library(${plugin_name} ${library_type})
    target_include_directories(${plugin_name}
            PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # set dependency to header_tool to allow generated code to compile
    add_dependencies(${plugin_name} header_tool)

    set_property(GLOBAL APPEND PROPERTY header_tool_targets ${plugin_name})
    setup_static_analysis(${module_name})

endfunction()

# populates the out_headers parameter with the engine headers list and the out_libs with the engine modules
# very useful to generate bindings for a scripting language
# do not use __linked_libs or __headers as they are variables set here in the macro
macro(get_engine_headers out_headers out_libs)
    get_target_property(__linked_libs bomb_engine_engine LINK_LIBRARIES)
    list(APPEND ${out_libs} ${__linked_libs})
    foreach(lib ${__linked_libs})
        # check if it is an engine module (starts with "bomb_engine")
        set(__engine_target)
        string(FIND ${lib} "bomb_engine" __engine_target)
        if (__engine_target LESS 0)
            # not an engine module, we don't care about it
            continue()
        endif ()
        get_target_property(__headers ${lib} HEADER_SET_HEADERS)
        list(APPEND ${out_headers} ${__headers})
    endforeach()
endmacro()

# macro for the header_tool. prepares the input arguments for the tool using
# the custom property header_tool_targets (containing every engine module and plugin)
macro(get_header_tool_targets out_ht_args)
    get_property(ht_targets GLOBAL PROPERTY header_tool_targets)
    foreach(ht_target ${ht_targets})
        get_target_property(__headers ${ht_target} HEADER_SET_HEADERS)
        if(NOT __headers STREQUAL "__headers-NOTFOUND")
            list(APPEND ${out_ht_args} "--module=${ht_target}")
            list(APPEND ${out_ht_args} ${__headers})
        endif()
    endforeach()
endmacro()

macro(setup_static_analysis target)
    if(STATIC_ANALYSIS)
        set_target_properties(${target} PROPERTIES
                VS_GLOBAL_RunCodeAnalysis true
                # Use clangtidy
                VS_GLOBAL_EnableClangTidyCodeAnalysis true
        )
        set(CMAKE_CXX_CLANG_TIDY clang-tidy)
        message("Static Analysis Enabled")
    endif ()
endmacro()