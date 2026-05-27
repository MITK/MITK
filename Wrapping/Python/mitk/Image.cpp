/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "GeometryHelpers.h"
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

py::array BuildNumpyArray(const Image& img, const void* data, bool writeable, py::capsule capsule)
{
  auto dtype = PixelTypeToDType(img.GetPixelType());
  auto format = PixelTypeFormat(img.GetPixelType());
  auto shape = ComputeNumpyShape(img);

  std::vector<py::ssize_t> strides(shape.size());
  strides.back() = dtype.itemsize();
  for (py::ssize_t i = static_cast<py::ssize_t>(shape.size()) - 2; i >= 0; --i)
    strides[i] = strides[i + 1] * shape[i + 1];

  // pybind11's buffer_info stores a non-const void*. Mirror what the
  // library itself does in its typed buffer_info(const T*, ...) overload:
  // const_cast the pointer here and let the readonly flag enforce
  // const-ness at the Python boundary.
  py::array arr(
    py::buffer_info(
      const_cast<void*>(data),
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

  {
    py::gil_scoped_release release;

    if (writeable)
      holder->write = std::make_unique<ImageWriteAccessor>(&img);
    else
      holder->read = std::make_unique<ImageReadAccessor>(&img);
  }

  const void* data = writeable
    ? holder->GetMutableData()
    : holder->GetData();

  if (data == nullptr)
    mitkThrow() << "Could not access image data";

  auto capsule = py::capsule(holder.release(), "mitk.ImageAccessor", &DeleteImageAccessorCapsule);

  return BuildNumpyArray(img, data, writeable, std::move(capsule));
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

  // Apply geometry overrides to every time step so a 4D image is not left
  // with inconsistent per-time-step geometries.
  auto* tg = img->GetTimeGeometry();
  const auto numSteps = tg->CountTimeSteps();

  for (mitk::TimeStepType t = 0; t < numSteps; ++t)
  {
    if (spacing.has_value())
      SetSpacing(tg, *spacing, t);

    if (origin.has_value())
      SetOrigin(tg, *origin, t);

    if (direction.has_value())
      SetDirection(tg, *direction, t);
  }

  return img;
}

// ---------------------------------------------------------------------------
// IOUtil shortcuts on Image (WP-4)
// ---------------------------------------------------------------------------

Image::Pointer LoadImage(const std::string& path)
{
  Image::Pointer img;

  {
    py::gil_scoped_release release;
    img = mitk::IOUtil::Load<Image>(path);
  }

  if (img.IsNull())
    throw py::value_error("Could not load Image from: " + path);

  return img;
}

void SaveImage(const Image* img, const std::string& path)
{
  if (img == nullptr)
    throw py::value_error("Cannot save a null image");

  py::gil_scoped_release release;
  mitk::IOUtil::Save(img, path);
}

// ---------------------------------------------------------------------------
// Module init
// ---------------------------------------------------------------------------

void InitImage(py::module_& m)
{
  auto image_class = py::class_<Image, Image::Pointer>(m, "Image",
    R"(N-dimensional medical image with attached geometry and properties.

``mitk.Image`` wraps the C++ ``mitk::Image`` class. It stores pixel data of
arbitrary numeric type, supports up to four dimensions (three spatial plus
time), and carries a full geometry (spacing, origin, direction cosines)
plus a typed property dictionary.

Construction is overloaded by argument type:

- ``mitk.Image()`` constructs an empty image; call :py:meth:`initialize`
  before reading or writing pixel data.
- ``mitk.Image(path)`` loads from disk (accepts ``str`` or ``pathlib.Path``).
- ``mitk.Image(array, spacing=..., origin=..., direction=...)`` constructs
  an image from a copy of a NumPy array (the image owns its buffer).

Equivalent factory methods :py:meth:`from_numpy` and :py:meth:`load` are
also available.
)");

  image_class
    .def(py::init([]() { return Image::New(); }),
      R"(Construct an empty image.

The pixel buffer is not allocated until :py:meth:`initialize` is called.
)")
    .def(py::init([](const std::filesystem::path& path) {
        return LoadImage(path.string());
      }),
      py::arg("path"),
      R"(Load an image from a file path.

Args:
    path: Path to an image file. Accepts ``str``, ``pathlib.Path``, or any
        object with a ``__fspath__`` method.

Raises:
    ValueError: If the file cannot be loaded (unknown format, no reader
        available, or the path does not exist).

Examples:
    >>> import mitk
    >>> img = mitk.Image("input.nrrd")
)")
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
      R"(Construct an image from a NumPy array.

The array is copied; the resulting image owns its buffer. The NumPy array
is interpreted in C order (slowest-varying axis first). For 4D arrays,
the slowest axis becomes the time dimension.

Args:
    array: NumPy array with 2, 3, or 4 dimensions. Must be of a supported
        dtype (``uint8``, ``int8``, ``uint16``, ``int16``, ``uint32``,
        ``int32``, ``float32``, ``float64``).
    spacing: Optional ``(sx, sy, sz)`` voxel spacing. Defaults to
        ``(1, 1, 1)``.
    origin: Optional ``(ox, oy, oz)`` origin in world coordinates.
        Defaults to ``(0, 0, 0)``.
    direction: Optional 3x3 direction cosine matrix. Defaults to identity.
    copy: Reserved for future zero-copy support. Must currently be True.

Raises:
    TypeError: If ``array`` is not array-like.
    ValueError: If ``array.ndim`` is less than 2 or greater than 4, or if
        ``copy=False`` (not yet supported).

Examples:
    >>> import numpy as np
    >>> img = mitk.Image(np.zeros((64, 64, 64), dtype=np.float32),
    ...                  spacing=(1.0, 1.0, 2.5))
)")
    .def("initialize",
      [](Image& img, const py::object& dtype, const std::vector<unsigned int>& dims, unsigned int channels) {
        img.Initialize(MakePixelType(dtype), static_cast<unsigned int>(dims.size()), dims.data(), channels);
      },
      py::arg("dtype"),
      py::arg("dimensions"),
      py::arg("channels") = 1,
      R"(Initialize the image with the given pixel type and dimensions.

Args:
    dtype: NumPy dtype or any value accepted by :py:func:`make_pixel_type`
        (for example ``"float32"``, ``numpy.float32``, or a
        :py:class:`PixelType` instance).
    dimensions: List of dimension sizes in MITK order ``(x, y, z[, t])``.
    channels: Number of components per pixel. Default 1; use 3 for RGB,
        4 for RGBA.

Examples:
    >>> img = mitk.Image()
    >>> img.initialize("float32", [64, 64, 64])
    >>> img.initialize("uint8", [256, 256, 32], channels=3)
)")
    .def("initialize",
      [](Image& img, const PixelType& type, const std::vector<unsigned int>& dims, unsigned int channels) {
        img.Initialize(type, static_cast<unsigned int>(dims.size()), dims.data(), channels);
      },
      py::arg("type"),
      py::arg("dimensions"),
      py::arg("channels") = 1,
      R"(Initialize the image with a pre-built :py:class:`PixelType`.

Args:
    type: A :py:class:`PixelType` instance describing the per-pixel layout.
    dimensions: List of dimension sizes in MITK order ``(x, y, z[, t])``.
    channels: Number of components per pixel (default 1).
)")
    .def("get_dimension", py::overload_cast<>(&Image::GetDimension, py::const_),
      R"(Return the number of dimensions of the image.

Equivalent to :py:attr:`ndim`.
)")
    .def("get_dimension", py::overload_cast<int>(&Image::GetDimension, py::const_), py::arg("i"),
      R"(Return the size of the *i*-th dimension.

Args:
    i: Dimension index in MITK order (0 = x, 1 = y, 2 = z, 3 = t).

Returns:
    The size of the requested dimension, in voxels.
)")

    .def_property_readonly("ndim", [](const Image& img) { return img.GetDimension(); },
      "Number of dimensions of the image (2, 3, or 4).")
    .def_property_readonly("shape",
      [](const Image& img) {
        auto shape = ComputeNumpyShape(img);
        py::tuple result(shape.size());
        for (size_t i = 0; i < shape.size(); ++i)
          result[i] = shape[i];
        return result;
      },
      R"(Image shape in NumPy order.

For a 3D scalar image of MITK dimensions ``(x, y, z)`` this returns
``(z, y, x)``. For a multi-channel image an extra trailing axis is
appended for the per-pixel component count.
)")
    .def_property_readonly("dtype", [](const Image& img) { return PixelTypeToDType(img.GetPixelType()); },
      "NumPy dtype matching the MITK pixel type.")
    .def_property_readonly("array", [](Image& img) { return AsNumpyDirect(img, false, 0); },
      R"(Read-only NumPy view of the image buffer at time step 0.

Zero-copy. The view pins the image alive via a smart-pointer capsule.
Equivalent to ``img.as_numpy(writeable=False, time_step=0)``.
)")

    .def("as_numpy",
      [](Image& img, bool use_accessor, bool writeable, mitk::TimeStepType time_step) {
        if (use_accessor)
        {
          if (time_step != 0)
            throw py::value_error(
              "as_numpy: time_step != 0 is not supported with use_accessor=True; "
              "use use_accessor=False to select a specific time step");
          return AsNumpyAccessor(img, writeable);
        }
        return AsNumpyDirect(img, writeable, time_step);
      },
      py::arg("use_accessor") = false,
      py::arg("writeable") = true,
      py::arg("time_step") = 0,
      R"(Return a NumPy view of the image buffer.

Writeable by default (matching NumPy convention). For a read-only view
pass ``writeable=False``, use :py:attr:`array`, or wrap with
``numpy.asarray(img)``.

By default the view is *direct*: zero-copy, no locking. The view pins the
image alive via a smart-pointer capsule. Pass ``use_accessor=True`` for a
view backed by ``ImageReadAccessor`` / ``ImageWriteAccessor``, which
acquires MITK's read/write lock and releases it when the NumPy array is
garbage-collected. Use the accessor-backed mode when other threads
(typically C++) may access the image concurrently.

Args:
    use_accessor: If True, return a view backed by
        ``ImageReadAccessor``/``ImageWriteAccessor`` (lock-acquiring).
        Defaults to False (direct, unlocked).
    writeable: If True, return a writable view; otherwise read-only.
        Defaults to True.
    time_step: Time-step index for 4D images. Must be 0 when
        ``use_accessor=True``; for time-step-specific access, use
        ``use_accessor=False``. Defaults to 0.

Returns:
    NumPy array sharing memory with the image buffer.

Raises:
    mitk.Exception: If the image data cannot be accessed.
    ValueError: If ``use_accessor=True`` and ``time_step != 0``.

Examples:
    >>> arr = img.as_numpy()
    >>> arr[0, 0, 0] = 1.0
    >>> # Concurrency-safe write:
    >>> locked = img.as_numpy(use_accessor=True, writeable=True)
    >>> locked[5, 5, 5] = 7
    >>> del locked  # release the write lock
)")

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
      py::arg("copy") = py::none(),
      R"(NumPy array protocol hook.

Allows ``numpy.asarray(img)`` and ``numpy.array(img)`` to work directly on
an image. Returns a read-only direct view by default; passes ``dtype`` and
``copy`` through with the standard NumPy semantics.
)")

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
      py::arg("copy") = true,
      R"(Construct an image from a NumPy array (explicit factory).

Equivalent to ``mitk.Image(array, spacing=..., origin=..., direction=...,
copy=...)``. See the array-constructor overload of :py:class:`Image` for
argument details.

Returns:
    A new :py:class:`Image` wrapping the array contents.
)")

    .def_static("load",
      [](const std::filesystem::path& path) { return LoadImage(path.string()); },
      py::arg("path"),
      R"(Load an image from a file path (explicit factory).

Equivalent to ``mitk.Image(path)``.

Args:
    path: Path to an image file. Accepts ``str``, ``pathlib.Path``, or any
        object with a ``__fspath__`` method.

Returns:
    The loaded image.

Raises:
    ValueError: If the file cannot be loaded.
)")
    .def("save",
      [](const Image* img, const std::filesystem::path& path) { SaveImage(img, path.string()); },
      py::arg("path"),
      R"(Save the image to a file.

The output format is inferred from the file extension. Common formats:
``.nrrd``, ``.nii``, ``.nii.gz``, ``.mha``, ``.mhd``, ``.dcm``.

Args:
    path: Output file path. Accepts ``str``, ``pathlib.Path``, or any
        object with a ``__fspath__`` method.

Raises:
    ValueError: If the image is null or the format is not writable.
)");

  // IPropertyOwner methods.
  bind_property_owner(image_class);

  // Geometry helpers.
  // Image-specific: ndim and shape are bound manually because Image
  // has a numpy-compatible shape property and overloaded GetDimension().
  BindGeometryAccessors<decltype(image_class), Image, false>(image_class);

  image_class.def_property_readonly(
    "properties",
    [](Image& self)
    {
      py::module_ propertyViewModule = py::module_::import("mitk.property_view");
      py::object PropertyView = propertyViewModule.attr("PropertyView");
      return PropertyView(self);
    },
    R"(Live, mutable view of the image's properties.

Returns a :py:class:`mitk.property_view.PropertyView`, a ``MutableMapping``
backed by the image's underlying ``IPropertyOwner`` interface. Reading
delegates to :py:meth:`get_property`; writing and deletion delegate to
:py:meth:`set_property` / :py:meth:`remove_property` and may raise
:py:class:`PropertyNotOwnedError` for provided (read-only) properties.

Examples:
    >>> img.properties["DICOM.PatientName"] = "Doe^John"
    >>> for key in img.properties:
    ...     print(key, img.properties[key])
    >>> del img.properties["my.custom.key"]
)");
}
