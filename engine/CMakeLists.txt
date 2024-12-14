# declare the engine library using the module function, we need the same operations anyway
new_engine_module(bomb_engine_engine STATIC)

# add source files here that are common to all submodules (or in pch if appropriate)
target_sources(bomb_engine_engine
        PRIVATE "engine.cpp"
)

# special target that builds some common tools; it is linked to all the other targets and included in the main pch file.
# because of this, keep it as simple as possible!
add_subdirectory(tools)

# The engine code is organized into libraries that deal with specific parts (ex. graphics, scripting, ...)
# add here all the subdirectories
add_engine_submodule(bomb_engine_engine utilities)
add_engine_submodule(bomb_engine_engine graphics)
add_engine_submodule(bomb_engine_engine script)
add_engine_submodule(bomb_engine_engine app)
add_subdirectory(header_tool)