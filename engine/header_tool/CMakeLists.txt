# HEADER TOOL - RUST

set(ht_targets_)
get_header_tool_targets(ht_targets_)

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
set_property(GLOBAL PROPERTY header_tool_products_dir ${generated_dir})

# this step isn't that necessary because cargo run will also compile if it detects changes,
# but i keep it anyway so I remember how to do it with other exes
#add_custom_target(header_tool_build WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMMAND cargo build -r --quiet  --target-dir ${CMAKE_CURRENT_BINARY_DIR})

# working directory is in binary dir, but target folder might exist from vscode development testing, so it is excluded from git just in case
add_custom_target(header_tool
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} 
	COMMAND cargo run -r --quiet --target-dir ${CMAKE_CURRENT_BINARY_DIR} --manifest-path "${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml" -- ${ht_targets_}
	BYPRODUCTS generated_dir
)

add_custom_command(OUTPUT generated_dir WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMAND header_tool)

add_dependencies(bomb_engine_engine header_tool)