new_engine_module(bomb_engine_scene OBJECT)

target_sources(bomb_engine_scene 
PRIVATE
	"scene.h"
	"scene.cpp"
)

find_package(EnTT REQUIRED)
target_link_libraries(bomb_engine_scene EnTT::EnTT)