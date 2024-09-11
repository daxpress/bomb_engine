# declare the engine library using the module function, we need the sdame operations anyway
new_engine_module(bomb_engine_engine STATIC)

target_sources(bomb_engine_engine PRIVATE "engine.cpp")

# The engine code is organized into libraries that deal with specific parts (ex. graphics, scripting, ...)
# add here all the subdirectories 
add_engine_submodule(bomb_engine_engine utilities)
add_engine_submodule(bomb_engine_engine graphics)
add_engine_submodule(bomb_engine_engine script)
add_engine_submodule(bomb_engine_engine app)
add_subdirectory(header_tool)