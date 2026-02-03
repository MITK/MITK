/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <pybind11/pybind11.h>
#include <mitkVector.h>

namespace py = pybind11;
using namespace mitk;

void InitVector2D(py::module_& m)
{
  py::class_<Vector2D>(m, "Vector2D")
    .def(py::init<>())
    .def(py::init<ScalarType, ScalarType>(), py::arg("x"), py::arg("y"))
    .def("__getitem__", [](const Vector2D& p, size_t i) { return p[i]; })
    .def("__setitem__", [](Vector2D& p, size_t i, ScalarType v) { p[i] = v; })
    .def_property("x",
      [](const Vector2D& p) { return p[0]; },
      [](Vector2D& p, ScalarType v) { p[0] = v; })
    .def_property("y",
      [](const Vector2D& p) { return p[1]; },
      [](Vector2D& p, ScalarType v) { p[1] = v; })
    .def("__repr__",
      [](const Vector2D& p) {
        return "<Vector2D ["
          + std::to_string(p[0]) + ", "
          + std::to_string(p[1]) + "]>";
      });
}

void InitVector3D(py::module_& m)
{
  py::class_<Vector3D>(m, "Vector3D")
    .def(py::init<>())
    .def(py::init<ScalarType, ScalarType, ScalarType>(), py::arg("x"), py::arg("y"), py::arg("z"))
    .def("__getitem__", [](const Vector3D& p, size_t i) { return p[i]; })
    .def("__setitem__", [](Vector3D& p, size_t i, ScalarType v) { p[i] = v; })
    .def_property("x",
      [](const Vector3D& p) { return p[0]; },
      [](Vector3D& p, ScalarType v) { p[0] = v; })
    .def_property("y",
      [](const Vector3D& p) { return p[1]; },
      [](Vector3D& p, ScalarType v) { p[1] = v; })
    .def_property("z",
      [](const Vector3D& p) { return p[2]; },
      [](Vector3D& p, ScalarType v) { p[2] = v; })
    .def("__repr__",
      [](const Vector3D& p) {
        return "<Vector3D ["
          + std::to_string(p[0]) + ", "
          + std::to_string(p[1]) + ", "
          + std::to_string(p[2]) + "]>";
      });
}

void InitVectors(py::module_& m)
{
  InitVector2D(m);
  InitVector3D(m);
}
