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
#include <mitkPreferenceListReaderOptionsFunctor.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace mitk;

// Note: only mitk::Image is wrapped at this stage of the bindings, so
// IOUtil.load() filters its results to Image instances. As more BaseData
// subclasses get bound (e.g. MultiLabelSegmentation in WP-10), this load
// function will widen to include them.

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

        std::vector<Image::Pointer> images;
        images.reserve(results.size());

        for (auto& base : results)
        {
          if (auto* img = dynamic_cast<Image*>(base.GetPointer()))
            images.emplace_back(img);
        }

        return images;
      },
      py::arg("path"),
      py::arg("reader_preferences") = py::none(),
      py::arg("reader_blacklist") = py::none(),
      "Load a file. Optional `reader_preferences` / `reader_blacklist` are "
      "sequences of reader description strings; when either is given, a "
      "PreferenceListReaderOptionsFunctor is built and passed to the C++ "
      "loader so the caller can steer reader selection. Currently returns "
      "only mitk.Image instances; other BaseData subclasses will be added "
      "as their bindings land.")
    .def_static("save",
      [](const Image* img, const std::string& path) {
        if (img == nullptr)
          throw py::value_error("Cannot save a null image");
        IOUtil::Save(img, path);
      },
      py::arg("data"), py::arg("path"),
      "Save an mitk.Image to disk. Other BaseData subclasses will be "
      "added as their bindings land.");
}
