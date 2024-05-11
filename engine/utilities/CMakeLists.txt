new_engine_module(bomb_engine_utilities STATIC)

target_sources(bomb_engine_utilities
PUBLIC 
"bomb_engine.h" "file_helper.h"
PRIVATE 
"file_helper.cpp"
)
