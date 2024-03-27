project(bomb_engine_editor)

add_executable (bomb_engine_editor)

target_sources(bomb_engine_editor PRIVATE "editor.cpp" "editor.h")

target_link_libraries(bomb_engine_editor PUBLIC bomb_engine_core)
target_include_directories(bomb_engine_editor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# use the following to copy the dependant DLLs to the target folder

add_custom_command(TARGET bomb_engine_editor POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  $<TARGET_RUNTIME_DLLS:bomb_engine_editor>
  $<TARGET_FILE_DIR:bomb_engine_editor>
  COMMAND_EXPAND_LISTS
  )