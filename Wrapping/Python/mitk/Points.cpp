/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <pybind11/pybind11.h>
#include <mitkPoint.h>

namespace py = pybind11;
using namespace mitk;

void InitPoint2D(py::module_& m)
{
  py::class_<Point2D>(m, "Point2D")
    .def(py::init<>())
    .def(py::init<ScalarType, ScalarType>(), py::arg("x"), py::arg("y"))
    .def("__getitem__", [](const Point2D& p, size_t i) { return p[i]; })
    .def("__setitem__", [](Point2D& p, size_t i, ScalarType v) { p[i] = v; })
    .def_property("x",
      [](const Point2D& p) { return p[0]; },
      [](Point2D& p, ScalarType v) { p[0] = v; })
    .def_property("y",
      [](const Point2D& p) { return p[1]; },
      [](Point2D& p, ScalarType v) { p[1] = v; })
    .def("__repr__",
      [](const Point2D& p) {
        return "<Point2D ["
          + std::to_string(p[0]) + ", "
          + std::to_string(p[1]) + "]>";
      });
}

void InitPoint3D(py::module_& m)
{
  py::class_<Point3D>(m, "Point3D")
    .def(py::init<>())
    .def(py::init<ScalarType, ScalarType, ScalarType>(), py::arg("x"), py::arg("y"), py::arg("z"))
    .def("__getitem__", [](const Point3D& p, size_t i) { return p[i]; })
    .def("__setitem__", [](Point3D& p, size_t i, ScalarType v) { p[i] = v; })
    .def_property("x",
      [](const Point3D& p) { return p[0]; },
      [](Point3D& p, ScalarType v) { p[0] = v; })
    .def_property("y",
      [](const Point3D& p) { return p[1]; },
      [](Point3D& p, ScalarType v) { p[1] = v; })
    .def_property("z",
      [](const Point3D& p) { return p[2]; },
      [](Point3D& p, ScalarType v) { p[2] = v; })
    .def("__repr__",
      [](const Point3D& p) {
        return "<Point3D ["
          + std::to_string(p[0]) + ", "
          + std::to_string(p[1]) + ", "
          + std::to_string(p[2]) + "]>";
      });
}

void InitPoints(py::module_& m)
{
  InitPoint2D(m);
  InitPoint3D(m);
}
