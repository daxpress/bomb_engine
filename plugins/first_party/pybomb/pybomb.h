#pragma once

namespace pybind11
{
class scoped_interpreter;
}

// use this to start the python interpreter
auto get_interpreter() -> pybind11::scoped_interpreter&;
