import sys

# we receive the list of header files that need to be parsed and bound with pybind11 syntax
# for debug purposes atm we simply print the list as is, the we'll start implementing the cool stuff
header_files = sys.argv[1:]
print(f"printing libs: {header_files}")