#include <pybind11/pybind11.h>

namespace py = pybind11;

// Declared by generated file!!!
void init_pybomb(py::module& m)
{
    // testing module functionality
    m.def("print", [](){Log(LogTempCategory, LogSeverity::Log, "python print using module");});
}