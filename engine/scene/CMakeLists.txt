new_engine_module(bomb_engine_scene OBJECT)

target_sources(bomb_engine_scene 
PRIVATE
	"scene.cpp" "entity.cpp"
PRIVATE
FILE_SET HEADERS FILES
	"scene.h" "entity.h" 
)

find_package(EnTT CONFIG REQUIRED)
target_link_libraries(bomb_engine_scene EnTT::EnTT bomb_engine_script)
