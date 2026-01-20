/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "PixelType.h"
#include "SmartPointer.h"

#include <mitkImage.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

#include <pybind11/numpy.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace mitk;

struct ImageAccessorHolder
{
  std::unique_ptr<ImageReadAccessor> read;
  std::unique_ptr<ImageWriteAccessor> write;

  const void* GetData() const
  {
    if (read) return read->GetData();
    if (write) return write->GetData();
    return nullptr;
  }

  void* GetMutableData()
  {
    if (write) return write->GetData();
    return nullptr;
  }
};

void DeleteImageAccessorCapsule(PyObject* capsule)
{
  auto ptr = PyCapsule_GetPointer(capsule, "mitk.ImageAccessor");
  delete static_cast<ImageAccessorHolder*>(ptr);
}

py::dtype PixelTypeToDType(const PixelType& pt)
{
  switch (pt.GetComponentType())
  {
    case itk::IOComponentEnum::UCHAR: return py::dtype::of<unsigned char>();
    case itk::IOComponentEnum::CHAR: return py::dtype::of<char>();
    case itk::IOComponentEnum::USHORT: return py::dtype::of<unsigned short>();
    case itk::IOComponentEnum::SHORT: return py::dtype::of<short>();
    case itk::IOComponentEnum::UINT: return py::dtype::of<unsigned int>();
    case itk::IOComponentEnum::INT: return py::dtype::of<int>();
    case itk::IOComponentEnum::FLOAT: return py::dtype::of<float>();
    case itk::IOComponentEnum::DOUBLE: return py::dtype::of<double>();
    default: mitkThrow() << "Unsupported pixel type: " << pt.GetComponentTypeAsString();
  }
}

std::string PixelTypeFormat(const PixelType& pt)
{
  switch (pt.GetComponentType())
  {
    case itk::IOComponentEnum::UCHAR: return py::format_descriptor<unsigned char>::format();
    case itk::IOComponentEnum::CHAR: return py::format_descriptor<char>::format();
    case itk::IOComponentEnum::USHORT: return py::format_descriptor<unsigned short>::format();
    case itk::IOComponentEnum::SHORT: return py::format_descriptor<short>::format();
    case itk::IOComponentEnum::UINT: return py::format_descriptor<unsigned int>::format();
    case itk::IOComponentEnum::INT: return py::format_descriptor<int>::format();
    case itk::IOComponentEnum::FLOAT: return py::format_descriptor<float>::format();
    case itk::IOComponentEnum::DOUBLE: return py::format_descriptor<double>::format();
    default: mitkThrow() << "Unsupported pixel type: " << pt.GetComponentTypeAsString();
  }
}

std::vector<py::ssize_t> ComputeNumpyShape(const Image& img)
{
  std::vector<py::ssize_t> shape;

  for (unsigned int i = 0; i < img.GetDimension(); ++i)
    shape.push_back(img.GetDimension(i));

  if (img.GetPixelType().GetNumberOfComponents() > 1)
    shape.push_back(img.GetPixelType().GetNumberOfComponents());

  std::reverse(shape.begin(), shape.end());

  return shape;
}

void InitImage(py::module_& m)
{
  py::class_<Image, Image::Pointer>(m, "Image")
    .def_static("new", &Image::New)
    .def("initialize",
      [](Image& img, const py::object& dtype, const std::vector<unsigned int>& dims, unsigned int channels) {
        img.Initialize(MakePixelType(dtype), static_cast<unsigned int>(dims.size()), dims.data(), channels);
      },
      py::arg("dtype"),
      py::arg("dimensions"),
      py::arg("channels") = 1)
    .def("initialize",
      [](Image& img, const PixelType& type, const std::vector<unsigned int>& dims, unsigned int channels) {
        img.Initialize(type, static_cast<unsigned int>(dims.size()), dims.data(), channels);
      },
      py::arg("type"),
      py::arg("dimensions"),
      py::arg("channels") = 1)
    .def("get_dimension", py::overload_cast<>(&Image::GetDimension, py::const_))
    .def("get_dimension", py::overload_cast<int>(&Image::GetDimension, py::const_), py::arg("i"))
    .def("as_numpy",
      [](Image& img, bool writeable) {
        auto holder = std::make_unique<ImageAccessorHolder>();

        if (writeable)
        {
          holder->write = std::make_unique<ImageWriteAccessor>(&img);
        }
        else
        {
          holder->read = std::make_unique<ImageReadAccessor>(&img);
        }

        auto data = writeable
          ? holder->GetMutableData()
          : const_cast<void*>(holder->GetData());

        if (data == nullptr)
          mitkThrow() << "Could not access image data";

        auto dtype = PixelTypeToDType(img.GetPixelType());
        auto format = PixelTypeFormat(img.GetPixelType());
        auto shape = ComputeNumpyShape(img);

        std::vector<py::ssize_t> strides(shape.size());
        strides.back() = dtype.itemsize();
        for (py::ssize_t i = shape.size() - 2; i >= 0; --i)
          strides[i] = strides[i + 1] * shape[i + 1];

        auto capsule = py::capsule(
          holder.release(),
          "mitk.ImageAccessor",
          &DeleteImageAccessorCapsule
        );

        py::array arr(
          py::buffer_info(
            data,
            dtype.itemsize(),
            format,
            shape.size(),
            shape,
            strides,
            !writeable
          ),
          capsule
        );

        if (!writeable)
          arr.attr("flags").attr("writeable") = false;

        return arr;
      },
      py::arg("writeable") = false);
}
