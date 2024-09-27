project(bomb_engine_tools)

add_library(bomb_engine_tools OBJECT)

# set dependency to header_tool to allow generated code to compile
add_dependencies(bomb_engine_tools header_tool)

set_property(GLOBAL APPEND PROPERTY header_tool_targets bomb_engine_tools)

target_sources(bomb_engine_tools
        PUBLIC
        FILE_SET HEADERS FILES
        "log.h" "dispatcher.h"
        PRIVATE
        "log.cpp" "dispatcher.cpp"
)

target_include_directories(bomb_engine_tools
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

find_package(fmt REQUIRED)
target_link_libraries(bomb_engine_tools fmt::fmt)