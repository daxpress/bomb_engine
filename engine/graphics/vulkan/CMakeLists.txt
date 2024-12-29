add_target_subdirectory(bomb_engine_graphics PRIVATE)

target_sources(bomb_engine_graphics
        PRIVATE
        "api_vulkan.cpp" "api_vulkan_structs.cpp" "api_vulkan_internal.h" "vulkan_buffer.h" "vulkan_buffer.cpp"
        "api_vulkan.h" "api_vulkan_structs.h" "vulkan_statics.h" "vulkan_statics.cpp" "vulkan_image.h" "vulkan_image.cpp"
        PRIVATE
        FILE_SET HEADERS FILES

)