/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "SmartPointer.h"

#include <mitkIOUtil.h>
#include <mitkImage.h>
#include <mitkLabelSetImage.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <filesystem>

namespace py = pybind11;
using namespace mitk;

// IOUtil.load() returns mitk::Image and mitk::MultiLabelSegmentation instances.
// Other BaseData subclasses not yet bound in Python are silently skipped.
// MultiLabelSegmentation is checked first in the dispatch loop because it is
// the more specific type (it does NOT inherit from Image).

void InitIOUtil(py::module_& m)
{
  py::class_<IOUtil>(m, "IOUtil",
    R"(File I/O for MITK objects.

``IOUtil`` is a namespace-style class with static methods for loading and
saving the data types currently bound in Python (:py:class:`Image` and
:py:class:`MultiLabelSegmentation`). The wheel bundles auto-load modules
for the common image formats (NRRD, NIfTI, MetaImage, DICOM, and others);
no further setup is required.

The :py:class:`Image` constructor and the shortcut methods
``Image.load`` / ``Image.save`` ultimately call into this class.
)")
    .def_static("load",
      [](const std::filesystem::path& path,
         std::optional<std::vector<std::string>> reader_preferences,
         std::optional<std::vector<std::string>> reader_blacklist) {
        const auto path_str = path.string();
        std::vector<BaseData::Pointer> results;

        if (reader_preferences.has_value() || reader_blacklist.has_value())
        {
          PreferenceListReaderOptionsFunctor functor(
            reader_preferences.value_or(std::vector<std::string>{}),
            reader_blacklist.value_or(std::vector<std::string>{}));
          results = IOUtil::Load(path_str, &functor);
        }
        else
        {
          results = IOUtil::Load(path_str);
        }

        py::list out;

        for (auto& base : results)
        {
          if (auto* seg = dynamic_cast<MultiLabelSegmentation*>(base.GetPointer()))
            out.append(MultiLabelSegmentation::Pointer(seg));
          else if (auto* img = dynamic_cast<Image*>(base.GetPointer()))
            out.append(Image::Pointer(img));
          // other BaseData subtypes not yet bound are silently skipped
        }

        return out;
      },
      py::arg("path"),
      py::arg("reader_preferences") = py::none(),
      py::arg("reader_blacklist") = py::none(),
      R"(Load a file and return a list of bound objects.

Each element of the returned list is either an :py:class:`Image` or a
:py:class:`MultiLabelSegmentation`, depending on what the MITK I/O system
chose to produce. ``BaseData`` subclasses that are not yet exposed to
Python are silently skipped.

Reader selection can be steered with ``reader_preferences`` (preferred
reader description strings) and ``reader_blacklist`` (readers to skip).
When either is provided, a ``PreferenceListReaderOptionsFunctor`` is
built and passed to the underlying C++ loader.

Args:
    path: Path to the file or directory to load. Accepts ``str``,
        ``pathlib.Path``, or any object with a ``__fspath__`` method.
        DICOM series accept a directory path.
    reader_preferences: Optional list of reader description strings to
        prefer.
    reader_blacklist: Optional list of reader description strings to
        skip.

Returns:
    A list of loaded objects (possibly empty).

Examples:
    >>> [img] = mitk.IOUtil.load("input.nrrd")
    >>> objs = mitk.IOUtil.load("DICOMSeries/")
)")
    .def_static("save",
      [](py::object data, const std::filesystem::path& path) {
        const BaseData* base = nullptr;
        if (py::isinstance<MultiLabelSegmentation>(data))
          base = data.cast<MultiLabelSegmentation*>();
        else if (py::isinstance<Image>(data))
          base = data.cast<Image*>();
        else
          throw py::type_error("data must be an mitk.Image or mitk.MultiLabelSegmentation");
        if (base == nullptr)
          throw py::value_error("Cannot save a null object");
        IOUtil::Save(base, path.string());
      },
      py::arg("data"), py::arg("path"),
      R"(Save an MITK object to disk.

The output format is inferred from the file extension.

Args:
    data: An :py:class:`Image` or :py:class:`MultiLabelSegmentation`.
    path: Output file path. Accepts ``str``, ``pathlib.Path``, or any
        object with a ``__fspath__`` method.

Raises:
    TypeError: If ``data`` is not a supported type.
    ValueError: If ``data`` is null.
)");
}
