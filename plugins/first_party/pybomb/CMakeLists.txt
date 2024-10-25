project(pybomb LANGUAGES CXX)

# specify the Python version to be 3.12 (atm at least)
find_package(Python 3.12 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)
add_library(pybomb STATIC)

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

# try and figure out the possible generated files
set(headers)
get_engine_headers(headers _)

set(generated_files)
foreach (header ${headers})
    get_filename_component(filename ${header} NAME_WE)
    set(pyheader ${generated_dir} "py_" ${filename} ".cpp")
    list(APPEND generated_files ${pyheader})
endforeach ()
# add the header for the module inits
set(pyheader ${generated_dir}module_inits.cpp)
list(APPEND generated_files ${pyheader})

set_source_files_properties(${generated_files} PROPERTIES GENERATED TRUE)

add_custom_target(pybinder
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND cargo run -r --quiet --target-dir ${CMAKE_CURRENT_BINARY_DIR} --manifest-path "${CMAKE_CURRENT_SOURCE_DIR}/pybinder/Cargo.toml" -- ${ht_products}
        DEPENDS header_tool
)

add_custom_command(OUTPUT generated_dir WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} COMMAND pybinder)

# Set dependency for pybinder: it needs the header generated files to compile!
#add_dependencies(pybomb_module pybinder)
add_dependencies(pybomb pybinder)

# ===================================================
# PYBOMB
# ===================================================


target_sources(pybomb
        PUBLIC
        "pybomb.h"
        PRIVATE
        ${generated_files}
        "pybomb.cpp"
)
target_include_directories(pybomb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${generated_dir})
# Use the following to embed the interpreter in the library
target_link_libraries(pybomb PRIVATE pybind11::embed bomb_engine_engine)

# Setting dependency on pybomb_module mostly to set the order and tell to build all by only referring to pybomb
#add_dependencies(pybomb pybomb_module)
