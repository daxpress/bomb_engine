project(pybomb_cpp LANGUAGES CXX)

# specify the Python version to be 3.13 (atm at least)
set(Python_FIND_ABI "ANY" "ANY" "ANY" "ON")
find_package(Python 3.13 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)
add_library(pybomb_cpp STATIC)

# ==================================================
# PYBINDER
# ==================================================

# getting the engine headers to pass them to the script that generates binding code
set(headers)
set(libs)
get_engine_headers(headers libs)

# get the location of the header tool products
get_property(ht_products GLOBAL PROPERTY header_tool_products_dir)

# set the pybinder generated dir
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated/")

add_custom_target(pybinder
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND cargo run -r --quiet --target-dir ${CMAKE_CURRENT_BINARY_DIR} --manifest-path "${CMAKE_CURRENT_SOURCE_DIR}/pybinder/Cargo.toml" -- ${ht_products}
        DEPENDS header_tool
)

add_custom_command(OUTPUT generated_dir WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMAND pybinder)

# ===================================================
# pybomb_cpp
# ===================================================

target_sources(pybomb_cpp
        PUBLIC
        "pybomb_cpp.h"
        PRIVATE
        "pybomb_cpp.cpp"
)
target_include_directories(pybomb_cpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${generated_dir})
# Use the following to embed the interpreter in the library
target_link_libraries(pybomb_cpp PRIVATE pybind11::embed)


# Set dependency for pybinder: it needs the header generated files to compile!
add_dependencies(pybomb_cpp pybinder)

# ===================================================
# pybomb module
# ===================================================

# try and figure out the possible generated files
set(headers)
get_engine_headers(headers _)

set(generated_files)
foreach (header ${headers})
    get_filename_component(filename ${header} NAME_WE)
    set(pyheader ${generated_dir}py_${filename}.cpp)
    list(APPEND generated_files ${pyheader})
endforeach ()
# add the header for the module inits
set(pyheader ${generated_dir}module_inits.cpp)
list(APPEND generated_files ${pyheader})

set_source_files_properties(${generated_files} PROPERTIES GENERATED TRUE)

pybind11_add_module(pybomb
        MODULE
        "pybomb.cpp"
        ${generated_files}
)

#target_compile_definitions(pybomb PRIVATE Py_GIL_DISABLED=1)
target_link_libraries(pybomb PRIVATE bomb_engine_engine)
target_include_directories(pybomb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${generated_dir})

# now generate the stubs after compiling pybomb
add_custom_command(TARGET pybomb POST_BUILD COMMAND stubgen -p pybomb -o .
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>" COMMENT "Generating stubs...")

add_dependencies(pybomb pybinder)
add_dependencies(pybomb_cpp pybomb)
