project(bomb_engine_editor)

add_executable (bomb_engine_editor)

target_sources(bomb_engine_editor PRIVATE  "editor.cpp" "editor.h")

target_link_libraries(bomb_engine_editor PRIVATE bomb_engine_engine)
target_link_libraries(bomb_engine_editor PRIVATE pybomb)
target_include_directories(bomb_engine_editor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# use the following to copy the dependant DLLs to the target folder

#add_custom_command(TARGET bomb_engine_editor POST_BUILD
#  COMMAND ${CMAKE_COMMAND} -E copy_if_different
#  $<TARGET_RUNTIME_DLLS:bomb_engine_editor>
#  $<TARGET_FILE_DIR:bomb_engine_editor>
#  COMMAND_EXPAND_LISTS
#  )

# copy the shaders to the editor folder so that we can easily test things while writing the core parts
# we'll eventually have a more refined build system
add_custom_command(
	TARGET bomb_engine_editor
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	$<TARGET_PROPERTY:bomb_engine_engine,SOURCE_DIR>/graphics/shaders
	$<TARGET_FILE_DIR:bomb_engine_editor>/shaders
)
add_custom_command(
	TARGET bomb_engine_editor
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	$<TARGET_PROPERTY:bomb_engine_engine,SOURCE_DIR>/graphics/assets
	$<TARGET_FILE_DIR:bomb_engine_editor>/assets
)