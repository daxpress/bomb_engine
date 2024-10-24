#include "pybomb.h"
#include <pybind11/embed.h>

namespace py = pybind11;

auto get_interpreter() -> py::scoped_interpreter&
{
    static py::scoped_interpreter python_interpreter{};
    return python_interpreter;

}

// PYBIND11_EMBEDDED_MODULE(pybomb, module)