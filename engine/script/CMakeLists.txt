new_engine_module(bomb_engine_script OBJECT)

target_sources(bomb_engine_script
PRIVATE
"scriptable.h" "native_script.h"
"native_script.cpp"
)

find_package(EnTT REQUIRED)
target_link_libraries(bomb_engine_script EnTT::EnTT)