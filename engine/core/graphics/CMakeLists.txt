add_engine_module(graphics PUBLIC)

target_sources(bomb_engine_core
PUBLIC "renderer.h"  "window.h" "api_bridge.h" "apis.h"
PRIVATE "renderer.cpp" "window.cpp" "api_bridge.cpp" 
)

add_subdirectory(api_bridge)