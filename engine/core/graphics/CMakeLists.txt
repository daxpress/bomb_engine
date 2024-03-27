add_engine_module(graphics)

target_sources(bomb_engine_core
PUBLIC "renderer.h"  "window.h" 
PRIVATE "renderer.cpp" "window.cpp")
