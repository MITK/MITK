/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkColorProperty.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace
{
  mitk::Color MakeColor(float r, float g, float b)
  {
    mitk::Color color;
    color[0] = r;
    color[1] = g;
    color[2] = b;
    return color;
  }
}

void InitColor(py::module_ &m)
{
  py::class_<mitk::Color>(m, "Color")
    .def(py::init<>())
    .def(py::init(&MakeColor), py::arg("r"), py::arg("g"), py::arg("b"))
    .def_property(
      "r", [](const mitk::Color &c) { return c.GetRed(); },
      [](mitk::Color &c, float v) { c.SetRed(v); })
    .def_property(
      "g", [](const mitk::Color &c) { return c.GetGreen(); },
      [](mitk::Color &c, float v) { c.SetGreen(v); })
    .def_property(
      "b", [](const mitk::Color &c) { return c.GetBlue(); },
      [](mitk::Color &c, float v) { c.SetBlue(v); })
    .def(
      "__getitem__",
      [](const mitk::Color &c, std::size_t i)
      {
        if (i >= 3)
          throw py::index_error();
        return c[i];
      })
    .def(
      "__setitem__",
      [](mitk::Color &c, std::size_t i, float v)
      {
        if (i >= 3)
          throw py::index_error();
        c[i] = v;
      })
    .def("__len__", [](const mitk::Color &) { return 3; })
    .def("__eq__", [](const mitk::Color &a, const mitk::Color &b) { return a == b; })
    .def("__repr__",
         [](const mitk::Color &c)
         {
           return "mitk.Color(" + std::to_string(c.GetRed()) + ", " + std::to_string(c.GetGreen()) + ", " +
                  std::to_string(c.GetBlue()) + ")";
         });
}
