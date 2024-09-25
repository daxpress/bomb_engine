new_engine_module(bomb_engine_utilities OBJECT)

target_sources(bomb_engine_utilities
        PRIVATE
        "file_helper.cpp"
        PRIVATE
        FILE_SET HEADERS FILES
        "file_helper.h"
)