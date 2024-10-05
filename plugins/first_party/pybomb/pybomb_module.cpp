#include <pybind11/pybind11.h>

namespace py = pybind11;

int add(int a, int b) { return a + b; }

PYBIND11_MODULE(pybomb, m)
{
    m.doc() = "pybomb binding test";
    m.def("add", &add, "adds two numbers", py::arg("a"), py::arg("b"));
}