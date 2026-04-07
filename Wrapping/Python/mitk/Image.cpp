/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "PixelType.h"
#include "SmartPointer.h"
#include "PropertyOwnerBindings.h"

#include <mitkAffineTransform3D.h>
#include <mitkBaseGeometry.h>
#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>
#include <mitkTimeGeometry.h>

#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <array>
#include <filesystem>
#include <optional>
#include <string>

namespace py = pybind11;
using namespace mitk;

// ---------------------------------------------------------------------------
// Capsule helpers
// ---------------------------------------------------------------------------

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

void DeleteImagePinCapsule(PyObject* capsule)
{
  auto ptr = PyCapsule_GetPointer(capsule, "mitk.ImagePin");
  delete static_cast<Image::Pointer*>(ptr);
}

// ---------------------------------------------------------------------------
// Pixel type / numpy dtype conversion
// ---------------------------------------------------------------------------

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

py::array BuildNumpyArray(const Image& img, void* data, bool writeable, py::capsule capsule)
{
  auto dtype = PixelTypeToDType(img.GetPixelType());
  auto format = PixelTypeFormat(img.GetPixelType());
  auto shape = ComputeNumpyShape(img);

  std::vector<py::ssize_t> strides(shape.size());
  strides.back() = dtype.itemsize();
  for (py::ssize_t i = static_cast<py::ssize_t>(shape.size()) - 2; i >= 0; --i)
    strides[i] = strides[i + 1] * shape[i + 1];

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
    arr.attr("flags").attr("writeable") = py::bool_(false);

  return arr;
}

py::array AsNumpyDirect(Image& img, bool writeable, mitk::TimeStepType time_step)
{
  void* data = img.GetData(static_cast<int>(time_step));

  if (data == nullptr)
    mitkThrow() << "Could not access image data at time step " << time_step;

  // Pin the Image::Pointer in the capsule so the buffer outlives the
  // numpy array regardless of who else holds a reference to the image.
  auto* pin = new Image::Pointer(&img);
  auto capsule = py::capsule(pin, "mitk.ImagePin", &DeleteImagePinCapsule);

  return BuildNumpyArray(img, data, writeable, std::move(capsule));
}

py::array AsNumpyAccessor(Image& img, bool writeable)
{
  auto holder = std::make_unique<ImageAccessorHolder>();

  if (writeable)
    holder->write = std::make_unique<ImageWriteAccessor>(&img);
  else
    holder->read = std::make_unique<ImageReadAccessor>(&img);

  void* data = writeable
    ? holder->GetMutableData()
    : const_cast<void*>(holder->GetData());

  if (data == nullptr)
    mitkThrow() << "Could not access image data";

  auto capsule = py::capsule(holder.release(), "mitk.ImageAccessor", &DeleteImageAccessorCapsule);

  return BuildNumpyArray(img, data, writeable, std::move(capsule));
}

// ---------------------------------------------------------------------------
// Geometry helpers (WP-1)
// ---------------------------------------------------------------------------

mitk::BaseGeometry::Pointer GetGeometryAtTimeStep(Image& img, mitk::TimeStepType timeStep)
{
  auto geom = img.GetTimeGeometry()->GetGeometryForTimeStep(timeStep);

  if (geom.IsNull())
    mitkThrow() << "Invalid time step: " << timeStep;

  return geom;
}

mitk::BaseGeometry::ConstPointer GetGeometryAtTimeStepConst(const Image& img, mitk::TimeStepType timeStep)
{
  auto geom = img.GetTimeGeometry()->GetGeometryForTimeStep(timeStep);

  if (geom.IsNull())
    mitkThrow() << "Invalid time step: " << timeStep;

  return mitk::BaseGeometry::ConstPointer(geom.GetPointer());
}

std::tuple<double, double, double> GetSpacingAt(const Image& img, mitk::TimeStepType t)
{
  auto geom = GetGeometryAtTimeStepConst(img, t);
  const auto s = geom->GetSpacing();
  return {s[0], s[1], s[2]};
}

void SetSpacingAt(Image& img, const std::array<double, 3>& spacing, mitk::TimeStepType t)
{
  auto geom = GetGeometryAtTimeStep(img, t);
  mitk::Vector3D v;
  v[0] = spacing[0];
  v[1] = spacing[1];
  v[2] = spacing[2];
  geom->SetSpacing(v);
}

std::tuple<double, double, double> GetOriginAt(const Image& img, mitk::TimeStepType t)
{
  auto geom = GetGeometryAtTimeStepConst(img, t);
  const auto o = geom->GetOrigin();
  return {o[0], o[1], o[2]};
}

void SetOriginAt(Image& img, const std::array<double, 3>& origin, mitk::TimeStepType t)
{
  auto geom = GetGeometryAtTimeStep(img, t);
  mitk::Point3D p;
  p[0] = origin[0];
  p[1] = origin[1];
  p[2] = origin[2];
  geom->SetOrigin(p);
}

py::array_t<double> ExtractDirection(const BaseGeometry& geom)
{
  const auto& matrix = geom.GetIndexToWorldTransform()->GetMatrix();
  const auto spacing = geom.GetSpacing();

  py::array_t<double> direction({static_cast<py::ssize_t>(3), static_cast<py::ssize_t>(3)});
  auto buf = direction.mutable_unchecked<2>();

  for (int row = 0; row < 3; ++row)
    for (int col = 0; col < 3; ++col)
      buf(row, col) = matrix[row][col] / spacing[col];

  return direction;
}

py::array_t<double> GetDirectionAt(const Image& img, mitk::TimeStepType t)
{
  return ExtractDirection(*GetGeometryAtTimeStepConst(img, t));
}

void SetDirectionAt(Image& img, py::array_t<double, py::array::c_style | py::array::forcecast> direction, mitk::TimeStepType t)
{
  if (direction.ndim() != 2 || direction.shape(0) != 3 || direction.shape(1) != 3)
    throw py::value_error("direction must be a 3x3 matrix");

  auto geom = GetGeometryAtTimeStep(img, t);
  auto buf = direction.unchecked<2>();
  const auto spacing = geom->GetSpacing();

  // Reconstruct the IndexToWorld matrix: each column = direction column * spacing
  auto* transform = geom->GetIndexToWorldTransform();
  auto matrix = transform->GetMatrix();

  for (int row = 0; row < 3; ++row)
    for (int col = 0; col < 3; ++col)
      matrix[row][col] = buf(row, col) * spacing[col];

  transform->SetMatrix(matrix);
  geom->Modified();
}

std::array<double, 9> GetDirectionCosinesAt(const Image& img, mitk::TimeStepType t)
{
  auto direction = GetDirectionAt(img, t);
  auto buf = direction.unchecked<2>();
  std::array<double, 9> flat{};

  for (int row = 0; row < 3; ++row)
    for (int col = 0; col < 3; ++col)
      flat[row * 3 + col] = buf(row, col);

  return flat;
}

// ---------------------------------------------------------------------------
// from_numpy (WP-2)
// ---------------------------------------------------------------------------

Image::Pointer ImageFromNumpy(py::array array,
                              std::optional<std::array<double, 3>> spacing,
                              std::optional<std::array<double, 3>> origin,
                              std::optional<py::array_t<double>> direction,
                              bool copy)
{
  if (!copy)
    throw py::value_error("from_numpy(copy=False) is not yet supported");

  // Make C-contiguous (forcecast to ensure compatible dtype passes through)
  auto arr = py::array::ensure(array, py::array::c_style);

  if (!arr)
    throw py::type_error("from_numpy: input is not array-like");

  const auto ndim = arr.ndim();

  if (ndim < 2 || ndim > 4)
    throw py::value_error("from_numpy: only 2D, 3D, and 4D arrays are supported");

  // numpy is C-order with the slowest dimension first; MITK expects
  // dimensions in (x, y, z[, t]) order with x as the fastest dimension.
  // Reverse the numpy shape to obtain the MITK dimensions vector.
  std::vector<unsigned int> dims(ndim);
  for (py::ssize_t i = 0; i < ndim; ++i)
    dims[i] = static_cast<unsigned int>(arr.shape(ndim - 1 - i));

  auto pixelType = MakePixelType(py::object(arr.dtype()));

  auto img = Image::New();
  img->Initialize(pixelType, static_cast<unsigned int>(ndim), dims.data());

  // Copy the buffer one volume at a time. For 2D/3D arrays this is a
  // single SetVolume call; for 4D the slowest numpy dim becomes time.
  if (ndim < 4)
  {
    img->SetVolume(arr.data());
  }
  else
  {
    const auto bytesPerVolume = static_cast<size_t>(arr.nbytes()) / dims[3];
    const auto* base = static_cast<const unsigned char*>(arr.data());
    for (unsigned int t = 0; t < dims[3]; ++t)
      img->SetVolume(base + static_cast<size_t>(t) * bytesPerVolume, static_cast<int>(t));
  }

  if (spacing.has_value())
    SetSpacingAt(*img, *spacing, 0);

  if (origin.has_value())
    SetOriginAt(*img, *origin, 0);

  if (direction.has_value())
    SetDirectionAt(*img, *direction, 0);

  return img;
}

// ---------------------------------------------------------------------------
// IOUtil shortcuts on Image (WP-4)
// ---------------------------------------------------------------------------

Image::Pointer LoadImage(const std::string& path)
{
  auto img = mitk::IOUtil::Load<Image>(path);

  if (img.IsNull())
    throw py::value_error("Could not load Image from: " + path);

  return img;
}

void SaveImage(const Image* img, const std::string& path)
{
  if (img == nullptr)
    throw py::value_error("Cannot save a null image");

  mitk::IOUtil::Save(img, path);
}

// ---------------------------------------------------------------------------
// Module init
// ---------------------------------------------------------------------------

void InitImage(py::module_& m)
{
  auto image_class = py::class_<Image, Image::Pointer>(m, "Image");
    // -------------------------------------------------------------- WP-3
    // Constructor overloads (pybind11 dispatches by argument type at call
  image_class
    // time). mitk.Image is the bound C++ class -- isinstance, type hints,
    // and IDE autocomplete all work normally.
    .def(py::init([]() { return Image::New(); }),
      "Construct an empty image. Call initialize(...) before use.")
    .def(py::init([](const std::filesystem::path& path) {
        return LoadImage(path.string());
      }),
      py::arg("path"),
      "Load an image from a file path. Accepts both str and pathlib.Path.")
    .def(py::init([](py::array array,
                     std::optional<std::array<double, 3>> spacing,
                     std::optional<std::array<double, 3>> origin,
                     std::optional<py::array_t<double>> direction,
                     bool copy) {
        return ImageFromNumpy(array, spacing, origin, direction, copy);
      }),
      py::arg("array"),
      py::arg("spacing") = py::none(),
      py::arg("origin") = py::none(),
      py::arg("direction") = py::none(),
      py::arg("copy") = true,
      "Construct an image from a numpy array, optionally with "
      "spacing/origin/direction overrides.")
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

    // -------------------------------------------------------------- WP-1
    // Per-time-step accessors
    .def("get_spacing",
      [](const Image& img, mitk::TimeStepType t) { return GetSpacingAt(img, t); },
      py::arg("time_step") = 0)
    .def("set_spacing",
      [](Image& img, const std::array<double, 3>& s, mitk::TimeStepType t) { SetSpacingAt(img, s, t); },
      py::arg("spacing"), py::arg("time_step") = 0)
    .def("get_origin",
      [](const Image& img, mitk::TimeStepType t) { return GetOriginAt(img, t); },
      py::arg("time_step") = 0)
    .def("set_origin",
      [](Image& img, const std::array<double, 3>& o, mitk::TimeStepType t) { SetOriginAt(img, o, t); },
      py::arg("origin"), py::arg("time_step") = 0)
    .def("get_direction",
      [](const Image& img, mitk::TimeStepType t) { return GetDirectionAt(img, t); },
      py::arg("time_step") = 0)
    .def("set_direction",
      [](Image& img, py::array_t<double, py::array::c_style | py::array::forcecast> d, mitk::TimeStepType t) {
        SetDirectionAt(img, d, t);
      },
      py::arg("direction"), py::arg("time_step") = 0)
    .def("get_direction_cosines",
      [](const Image& img, mitk::TimeStepType t) { return GetDirectionCosinesAt(img, t); },
      py::arg("time_step") = 0)
    .def("get_geometry",
      [](Image& img, mitk::TimeStepType t) { return GetGeometryAtTimeStep(img, t); },
      py::arg("time_step") = 0)

    // Convenience properties (always operate on time step 0)
    .def_property("spacing",
      [](const Image& img) { return GetSpacingAt(img, 0); },
      [](Image& img, const std::array<double, 3>& s) { SetSpacingAt(img, s, 0); })
    .def_property("origin",
      [](const Image& img) { return GetOriginAt(img, 0); },
      [](Image& img, const std::array<double, 3>& o) { SetOriginAt(img, o, 0); })
    .def_property("direction",
      [](const Image& img) { return GetDirectionAt(img, 0); },
      [](Image& img, py::array_t<double, py::array::c_style | py::array::forcecast> d) { SetDirectionAt(img, d, 0); })
    .def_property_readonly("direction_cosines", [](const Image& img) { return GetDirectionCosinesAt(img, 0); })
    .def_property_readonly("ndim", [](const Image& img) { return img.GetDimension(); })
    .def_property_readonly("shape",
      [](const Image& img) {
        auto shape = ComputeNumpyShape(img);
        py::tuple result(shape.size());
        for (size_t i = 0; i < shape.size(); ++i)
          result[i] = shape[i];
        return result;
      })
    .def_property_readonly("dtype", [](const Image& img) { return PixelTypeToDType(img.GetPixelType()); })
    .def_property_readonly("array", [](Image& img) { return AsNumpyDirect(img, false, 0); })
    .def_property_readonly("time_steps",
      [](const Image& img) { return img.GetTimeGeometry()->CountTimeSteps(); })
    .def_property_readonly("time_geometry",
      [](Image& img) { return mitk::TimeGeometry::Pointer(img.GetTimeGeometry()); })

    // -------------------------------------------------------------- direct + accessor as_numpy
    .def("as_numpy",
      [](Image& img, bool use_accessor, bool writeable, mitk::TimeStepType time_step) {
        if (use_accessor)
          return AsNumpyAccessor(img, writeable);
        return AsNumpyDirect(img, writeable, time_step);
      },
      py::arg("use_accessor") = false,
      py::arg("writeable") = true,
      py::arg("time_step") = 0,
      "Return a numpy view of the image. Writeable by default (numpy "
      "convention); pass writeable=False for an explicit read-only view. "
      "For an implicit read-only view, use img.array or "
      "np.asarray(img). By default uses direct, unlocked access (the "
      "numpy view pins the Image alive via a smart pointer capsule). "
      "Pass use_accessor=True to get the legacy "
      "ImageReadAccessor/ImageWriteAccessor-backed view, which acquires "
      "MITK's read/write lock and releases it when the numpy array is "
      "garbage-collected.")

    // -------------------------------------------------------------- WP-5
    .def("__array__",
      [](Image& img, py::object dtype, py::object copy) {
        auto arr = AsNumpyDirect(img, false, 0);

        if (!dtype.is_none())
          arr = arr.attr("astype")(dtype, py::arg("copy") = false);

        if (!copy.is_none() && copy.cast<bool>())
          arr = arr.attr("copy")();

        return arr;
      },
      py::arg("dtype") = py::none(),
      py::arg("copy") = py::none())

    // -------------------------------------------------------------- WP-2
    .def_static("from_numpy",
      [](py::array array,
         std::optional<std::array<double, 3>> spacing,
         std::optional<std::array<double, 3>> origin,
         std::optional<py::array_t<double>> direction,
         bool copy) {
        return ImageFromNumpy(array, spacing, origin, direction, copy);
      },
      py::arg("array"),
      py::arg("spacing") = py::none(),
      py::arg("origin") = py::none(),
      py::arg("direction") = py::none(),
      py::arg("copy") = true)

    // -------------------------------------------------------------- WP-4 shortcuts
    .def_static("load",
      [](const std::string& path) { return LoadImage(path); },
      py::arg("path"))
    .def("save",
      [](const Image* img, const std::string& path) { SaveImage(img, path); },
      py::arg("path"));


  // Bind property owner methods
  bind_property_owner<decltype(image_class), Image>(image_class);

  // Attach properties view
  image_class.attr("properties") = py::cpp_function(
    [](Image& self) {
      // Import PropertyView and create an instance
      py::module_ propertyViewModule = py::module_::import("mitk.property_view");
      py::object PropertyView = propertyViewModule.attr("PropertyView");
      return PropertyView(self);
    },
    py::return_value_policy::reference
  );
}
