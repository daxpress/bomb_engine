new_engine_module(bomb_engine_script STATIC)

target_sources(bomb_engine_script
PUBLIC 
"iscriptable.h" "native_script.h"
PRIVATE
"native_script.cpp"
)