/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "PropertyAutoWrap.h"
#include "SmartPointer.h"
#include <mitkPropertyList.h>
#include <nlohmann/json.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

// Helper function for set_property with auto-wrapping
static void setPropertyHelper(mitk::PropertyList &pl, const std::string &key, py::object value)
{
  auto prop = resolvePropertyValue(pl, key, value);
  pl.SetProperty(key, prop);
}

/**
 * \brief Initializes Python bindings for MITK PropertyList.
 *
 * Registers the PropertyList class with methods for property management
 * and JSON serialization via MITK's self-contained JSON format.
 */
void InitPropertyList(py::module_ &m)
{
  auto propertyList_class = py::class_<mitk::PropertyList, mitk::PropertyList::Pointer>(m, "PropertyList");

  propertyList_class.def(py::init([]() { return mitk::PropertyList::New(); }))
    .def(
      "get_property",
      [](const mitk::PropertyList &pl, const std::string &key) -> mitk::BaseProperty::Pointer
      {
        return pl.GetProperty(key); // returns nullptr -> None if not found
      },
      py::arg("key"))
    .def(
      "property_is_owned",
      [](const mitk::PropertyList &pl, const std::string &key)
      {
        // PropertyList owns all its properties -- True if key exists
        return pl.GetProperty(key) != nullptr;
      },
      py::arg("key"))
    .def("set_property", &setPropertyHelper, py::arg("key"), py::arg("value"))
    .def(
      "remove_property",
      [](mitk::PropertyList &pl, const std::string &key) { pl.RemoveProperty(key); },
      py::arg("key"))
    .def_property_readonly("property_keys",
                           [](const mitk::PropertyList &pl)
                           {
                             auto keys = pl.GetPropertyKeys();
                             return std::vector<std::string>(keys.begin(), keys.end());
                           })
    .def("to_json",
         [](const mitk::PropertyList &pl) { return mitk::ConvertPropertyListToSelfContainedJson(&pl).dump(); })
    .def_static(
      "from_json",
      [](const std::string &json)
      { return mitk::ConvertPropertyListFromSelfContainedJson(nlohmann::json::parse(json)); },
      py::arg("json"));

  // Attach properties view
  propertyList_class.def_property_readonly(
    "properties",
    [](mitk::PropertyList &self)
    {
      py::module_ propertyViewModule = py::module_::import("mitk.property_view");
      py::object PropertyView = propertyViewModule.attr("PropertyView");
      return PropertyView(self);
    },
    py::return_value_policy::reference);
}
