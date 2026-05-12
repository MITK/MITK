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

    // numpy bool maps to MITK's conventional unsigned char representation.
    // Check this before the generic char/uchar branches because numpy bool
    // is a distinct dtype with its own kind ('b').
    if (dt.kind() == 'b') return MakePixelType<unsigned char, unsigned char>(components);

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
  py::class_<PixelType>(m, "PixelType",
    R"(Describes the per-pixel layout of an :py:class:`Image`.

A ``PixelType`` combines a component scalar type (``uint8``, ``int8``,
``uint16``, ``int16``, ``uint32``, ``int32``, ``float32``, ``float64``)
with a component count (1 for scalar pixels, 3 for RGB, 4 for RGBA, etc.).

``PixelType`` instances are usually constructed indirectly via
:py:func:`make_pixel_type` or by passing a NumPy dtype to
:py:meth:`Image.initialize`.
)")
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
    [](const py::object& dtype, size_t components) { return MakePixelType(dtype, components); },
    py::arg("dtype"),
    py::arg("components") = 1,
    R"(Construct a :py:class:`PixelType` from a NumPy dtype.

Args:
    dtype: NumPy dtype or any value accepted by ``numpy.dtype()``
        (string names like ``"float32"``, scalar types like
        ``numpy.float32``, or an existing dtype object).
    components: Number of components per pixel. Defaults to 1.

Returns:
    A :py:class:`PixelType` matching the requested layout.

Raises:
    RuntimeError: If the dtype is not supported by MITK.

Examples:
    >>> pt = mitk.make_pixel_type("float32")
    >>> rgba = mitk.make_pixel_type("uint8", components=4)
)");
}
