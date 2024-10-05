#include "pybomb.h"
#include <pybind11/embed.h>

namespace py = pybind11;

py::scoped_interpreter& get_interpreter()
{
    static py::scoped_interpreter python_interpreter{};
    return python_interpreter;
}

// PYBIND11_EMBEDDED_MODULE(pybomb, module) {
// }