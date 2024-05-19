project(pybomb LANGUAGES CXX)

# getting the engine headers to pass them to the script that generates binding code
set(headers)
set(libs)
get_engine_headers(headers libs)