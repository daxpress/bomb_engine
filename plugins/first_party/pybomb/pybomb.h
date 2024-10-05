#pragma once

namespace pybind11
{
class scoped_interpreter;
}

// use this to start the python interpreter
pybind11::scoped_interpreter& get_interpreter();
