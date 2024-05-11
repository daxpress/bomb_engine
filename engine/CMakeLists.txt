# declare the engine library using the module function, we need the sdame operations anyway
new_engine_module(bomb_engine_engine STATIC)

# The engine code is organized into libraries that deal with specific parts (ex. graphics, scripting, ...)
# add here all the subdirectories 
add_engine_submodule(bomb_engine_engine utilities)
add_engine_submodule(bomb_engine_engine graphics)

# add external libraries here! it helps with keeping track of all the third party stuff
