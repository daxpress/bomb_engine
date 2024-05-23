import sys

import header_tool
import header_checker


def run():
    print("Running pybinder...")
    # we receive the list of header files that need to be parsed and bound with pybind11 syntax
    # for debug purposes atm we simply print the list as is, the we'll start implementing the cool stuff
    header_files = sys.argv[1:]
    #print(f"printing libs: {header_files}")
    header_cache = "header_cache.cache"
    # instantiate header checker to retrieve the headers that need to be generated/modified/removed
    print("Checking headers...")
    checker = header_checker.HeaderChecker(header_files, header_cache)
    (add, remove, change) = checker.result()
    print(f"New: {add.keys()}\nRemoved: {remove}\nChanged: {change.keys()}")
    print("Running Header Tool...")
    tool = header_tool.HeaderTool(add, remove, change)
    tool.run()
    print("All done.")

if __name__ == "__main__":
    run()