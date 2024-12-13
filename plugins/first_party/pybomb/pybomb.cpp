#include <pybind11/pybind11.h>

namespace py = pybind11;

// Declared by generated file!!!

class SomeClass
{
    public:
    SomeClass() = default;
    SomeClass(const std::string& name){};
};

void init_pybomb(py::module& m)
{
    // testing module functionality
    m.def("print", []() { Log(LogTempCategory, LogSeverity::Log, "python print using module"); });
    py::class_<SomeClass>(m, "SomeClass")
    .def(py::init<const std::string&>(), py::arg("name"));
}