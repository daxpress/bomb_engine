add_engine_module(core PUBLIC)

target_sources(bomb_engine_core
PUBLIC "bomb_engine.h"
PRIVATE )


add_subdirectory(graphics)
