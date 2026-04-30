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

namespace py = pybind11;
using namespace mitk;

// IOUtil.load() returns mitk::Image and mitk::MultiLabelSegmentation instances.
// Other BaseData subclasses not yet bound in Python are silently skipped.
// MultiLabelSegmentation is checked first in the dispatch loop because it is
// the more specific type (it does NOT inherit from Image).

void InitIOUtil(py::module_& m)
{
  py::class_<IOUtil>(m, "IOUtil")
    .def_static("load",
      [](const std::string& path,
         std::optional<std::vector<std::string>> reader_preferences,
         std::optional<std::vector<std::string>> reader_blacklist) {
        std::vector<BaseData::Pointer> results;

        if (reader_preferences.has_value() || reader_blacklist.has_value())
        {
          PreferenceListReaderOptionsFunctor functor(
            reader_preferences.value_or(std::vector<std::string>{}),
            reader_blacklist.value_or(std::vector<std::string>{}));
          results = IOUtil::Load(path, &functor);
        }
        else
        {
          results = IOUtil::Load(path);
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
      "Load a file and return a list of bound objects. Each element is either "
      "an mitk.Image or an mitk.MultiLabelSegmentation depending on what the "
      "MITK IO system loaded. Optional `reader_preferences` / `reader_blacklist` "
      "are sequences of reader description strings; when either is given, a "
      "PreferenceListReaderOptionsFunctor is built and passed to the C++ loader "
      "so the caller can steer reader selection. BaseData subclasses not yet "
      "bound in Python are silently skipped.")
    .def_static("save",
      [](py::object data, const std::string& path) {
        const BaseData* base = nullptr;
        if (py::isinstance<MultiLabelSegmentation>(data))
          base = data.cast<MultiLabelSegmentation*>();
        else if (py::isinstance<Image>(data))
          base = data.cast<Image*>();
        else
          throw py::type_error("data must be an mitk.Image or mitk.MultiLabelSegmentation");
        if (base == nullptr)
          throw py::value_error("Cannot save a null object");
        IOUtil::Save(base, path);
      },
      py::arg("data"), py::arg("path"),
      "Save an mitk.Image or mitk.MultiLabelSegmentation to disk.");
}
