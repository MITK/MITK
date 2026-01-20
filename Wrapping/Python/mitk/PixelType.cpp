/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "PixelType.h"
#include <pybind11/numpy.h>

namespace py = pybind11;
using namespace mitk;

namespace mitk
{
  PixelType MakePixelType(const py::object& dtype, size_t components)
  {
    auto dt = py::dtype::from_args(dtype);

    if (dt.is(py::dtype::of<unsigned char>())) return MakePixelType<unsigned char, unsigned char>(components);
    if (dt.is(py::dtype::of<char>())) return MakePixelType<char, char>(components);
    if (dt.is(py::dtype::of<unsigned short>())) return MakePixelType<unsigned short, unsigned short>(components);
    if (dt.is(py::dtype::of<short>())) return MakePixelType<short, short>(components);
    if (dt.is(py::dtype::of<unsigned int>())) return MakePixelType<unsigned int, unsigned int>(components);
    if (dt.is(py::dtype::of<int>())) return MakePixelType<int, int>(components);
    if (dt.is(py::dtype::of<float>())) return MakePixelType<float, float>(components);
    if (dt.is(py::dtype::of<double>())) return MakePixelType<double, double>(components);

    mitkThrow() << "Unsupported numpy dtype: " << std::string(py::str(dt));
  }
}

void InitPixelType(py::module_& m)
{
  py::class_<PixelType>(m, "PixelType")
    .def("__repr__", [](const PixelType& pt) {
      std::string repr = "<PixelType: ";

      const auto pixelTypeStr = pt.GetPixelTypeAsString();

      if (!pixelTypeStr.empty())
        repr += pixelTypeStr + " (";

      if (const auto numComponents = pt.GetNumberOfComponents(); numComponents > 1)
        repr += std::to_string(numComponents) + " * ";

      repr += pt.GetComponentTypeAsString();

      if (!pixelTypeStr.empty())
        repr += ")";

      repr += ">";

      return repr;
    });

  m.def("make_pixel_type",
    py::overload_cast<const py::object&, size_t>(&MakePixelType),
    py::arg("dtype"),
    py::arg("components") = 1);
}
