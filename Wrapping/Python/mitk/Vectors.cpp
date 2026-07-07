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
  py::class_<Vector2D>(m, "Vector2D",
    R"(2D vector in world coordinates.

A pair of double-precision scalars representing a direction or offset.
Distinct from :py:class:`Point2D`: a vector encodes a displacement and is
unaffected by translations of the coordinate system.
)")
    .def(py::init<>(),
      "Construct a zero vector (0, 0).")
    .def(py::init<ScalarType, ScalarType>(), py::arg("x"), py::arg("y"),
      R"(Construct a vector with the given components.

Args:
    x: X component.
    y: Y component.
)")
    .def("__getitem__", [](const Vector2D& p, size_t i) { return p[i]; },
      "Return the i-th component (0 = x, 1 = y).")
    .def("__setitem__", [](Vector2D& p, size_t i, ScalarType v) { p[i] = v; },
      "Set the i-th component.")
    .def_property("x",
      [](const Vector2D& p) { return p[0]; },
      [](Vector2D& p, ScalarType v) { p[0] = v; },
      "X component.")
    .def_property("y",
      [](const Vector2D& p) { return p[1]; },
      [](Vector2D& p, ScalarType v) { p[1] = v; },
      "Y component.")
    .def("__repr__",
      [](const Vector2D& p) {
        return "<Vector2D ["
          + std::to_string(p[0]) + ", "
          + std::to_string(p[1]) + "]>";
      });
}

void InitVector3D(py::module_& m)
{
  py::class_<Vector3D>(m, "Vector3D",
    R"(3D vector in world coordinates.

A triple of double-precision scalars representing a direction or offset.
Distinct from :py:class:`Point3D`: a vector encodes a displacement and is
unaffected by translations of the coordinate system.
)")
    .def(py::init<>(),
      "Construct a zero vector (0, 0, 0).")
    .def(py::init<ScalarType, ScalarType, ScalarType>(), py::arg("x"), py::arg("y"), py::arg("z"),
      R"(Construct a vector with the given components.

Args:
    x: X component.
    y: Y component.
    z: Z component.
)")
    .def("__getitem__", [](const Vector3D& p, size_t i) { return p[i]; },
      "Return the i-th component (0 = x, 1 = y, 2 = z).")
    .def("__setitem__", [](Vector3D& p, size_t i, ScalarType v) { p[i] = v; },
      "Set the i-th component.")
    .def_property("x",
      [](const Vector3D& p) { return p[0]; },
      [](Vector3D& p, ScalarType v) { p[0] = v; },
      "X component.")
    .def_property("y",
      [](const Vector3D& p) { return p[1]; },
      [](Vector3D& p, ScalarType v) { p[1] = v; },
      "Y component.")
    .def_property("z",
      [](const Vector3D& p) { return p[2]; },
      [](Vector3D& p, ScalarType v) { p[2] = v; },
      "Z component.")
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
