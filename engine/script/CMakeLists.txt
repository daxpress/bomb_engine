new_engine_module(bomb_engine_script OBJECT)

target_sources(bomb_engine_script
PRIVATE
	"native_script.cpp" "internal.h"
PRIVATE
FILE_SET HEADERS FILES
	"scriptable.h" "native_script.h"
)

find_package(EnTT REQUIRED)
target_link_libraries(bomb_engine_script EnTT::EnTT)