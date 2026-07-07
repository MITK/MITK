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
  py::class_<Point2D>(m, "Point2D",
    R"(2D point in world coordinates.

A pair of double-precision scalars accessible by index or by ``x`` / ``y``
attributes.

Examples:
    >>> p = mitk.Point2D(1.0, 2.0)
    >>> p.x, p.y
    (1.0, 2.0)
    >>> p[0]
    1.0
)")
    .def(py::init<>(),
      "Construct a point at the origin (0, 0).")
    .def(py::init<ScalarType, ScalarType>(), py::arg("x"), py::arg("y"),
      R"(Construct a point at the given coordinates.

Args:
    x: X coordinate.
    y: Y coordinate.
)")
    .def("__getitem__", [](const Point2D& p, size_t i) { return p[i]; },
      "Return the i-th coordinate (0 = x, 1 = y).")
    .def("__setitem__", [](Point2D& p, size_t i, ScalarType v) { p[i] = v; },
      "Set the i-th coordinate.")
    .def_property("x",
      [](const Point2D& p) { return p[0]; },
      [](Point2D& p, ScalarType v) { p[0] = v; },
      "X coordinate.")
    .def_property("y",
      [](const Point2D& p) { return p[1]; },
      [](Point2D& p, ScalarType v) { p[1] = v; },
      "Y coordinate.")
    .def("__repr__",
      [](const Point2D& p) {
        return "<Point2D ["
          + std::to_string(p[0]) + ", "
          + std::to_string(p[1]) + "]>";
      });
}

void InitPoint3D(py::module_& m)
{
  py::class_<Point3D>(m, "Point3D",
    R"(3D point in world coordinates.

A triple of double-precision scalars accessible by index or by ``x`` /
``y`` / ``z`` attributes.

Examples:
    >>> p = mitk.Point3D(1.0, 2.0, 3.0)
    >>> p.x, p.y, p.z
    (1.0, 2.0, 3.0)
)")
    .def(py::init<>(),
      "Construct a point at the origin (0, 0, 0).")
    .def(py::init<ScalarType, ScalarType, ScalarType>(), py::arg("x"), py::arg("y"), py::arg("z"),
      R"(Construct a point at the given coordinates.

Args:
    x: X coordinate.
    y: Y coordinate.
    z: Z coordinate.
)")
    .def("__getitem__", [](const Point3D& p, size_t i) { return p[i]; },
      "Return the i-th coordinate (0 = x, 1 = y, 2 = z).")
    .def("__setitem__", [](Point3D& p, size_t i, ScalarType v) { p[i] = v; },
      "Set the i-th coordinate.")
    .def_property("x",
      [](const Point3D& p) { return p[0]; },
      [](Point3D& p, ScalarType v) { p[0] = v; },
      "X coordinate.")
    .def_property("y",
      [](const Point3D& p) { return p[1]; },
      [](Point3D& p, ScalarType v) { p[1] = v; },
      "Y coordinate.")
    .def_property("z",
      [](const Point3D& p) { return p[2]; },
      [](Point3D& p, ScalarType v) { p[2] = v; },
      "Z coordinate.")
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
