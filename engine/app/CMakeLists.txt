new_engine_module(bomb_engine_app OBJECT)

target_sources(bomb_engine_app
	PRIVATE 
	"app.cpp" "time_manager.cpp"  "scene.cpp" "entity.cpp" "dispatcher.cpp"
	PRIVATE FILE_SET HEADERS FILES
	"app.h" "time_manager.h"
   "scene.h" "entity.h" "dispatcher.h"
)

find_package(EnTT CONFIG REQUIRED)

target_link_libraries(bomb_engine_app 
bomb_engine_graphics EnTT::EnTT bomb_engine_script)