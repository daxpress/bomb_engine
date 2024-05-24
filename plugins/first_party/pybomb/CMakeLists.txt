project(pybomb LANGUAGES CXX)

add_library(pybomb STATIC)

target_sources(pybomb PRIVATE "pybomb.cpp")

# getting the engine headers to pass them to the script that generates binding code
set(headers)
set(libs)
get_engine_headers(headers libs)

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
# declare the binder program as usual
add_executable(python_binder "python_binder/main.cpp")
# set a new target that depends on the binder build and generated files
add_custom_target(bind_phase DEPENDS python_binder generated_dir)
# add command that runs the binder after the binder is compiled to produce generated headers
add_custom_command(OUTPUT generated_dir WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMMAND python_binder headers)
# finally, add a dependency to bind_phase execution, so that it will compile pybomb after having the generated files
add_dependencies(pybomb bind_phase)