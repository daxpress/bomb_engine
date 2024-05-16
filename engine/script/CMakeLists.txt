new_engine_module(bomb_engine_script OBJECT)

target_sources(bomb_engine_script
PRIVATE
"iscriptable.h" "native_script.h"
"native_script.cpp"
)
