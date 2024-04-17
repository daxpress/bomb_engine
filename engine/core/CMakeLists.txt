add_engine_module(core PUBLIC)

target_sources(bomb_engine_core
PUBLIC 
"bomb_engine.h" "file_helper.h"
PRIVATE 
"file_helper.cpp"
)


add_subdirectory(graphics)
