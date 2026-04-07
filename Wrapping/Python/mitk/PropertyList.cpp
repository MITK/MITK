/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "PropertyAutoWrap.h"
#include "PropertyConversionUtils.h"
#include "SmartPointer.h"
#include <mitkPropertyList.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

// Use the centralized conversion utilities
using mitk::python::dictToProperty;
using mitk::python::propertyToDict;

/**
 * @brief Converts a PropertyList to a Python dictionary for JSON serialization
 *
 * @param pl The PropertyList to convert
 * @return Python dictionary representing the property list
 */
py::dict convertPropertyListToDict(const mitk::PropertyList &pl)
{
  py::dict result;
  auto keys = pl.GetPropertyKeys();

  for (const auto &key : keys)
  {
    auto prop = pl.GetProperty(key);
    if (prop)
    {
      // Use the centralized conversion utility
      result[py::str(key)] = propertyToDict(*prop);
    }
  }

  return result;
}

// Helper function to create PropertyList from dict
mitk::PropertyList::Pointer convertDictToPropertyList(const py::dict &d)
{
  auto pl = mitk::PropertyList::New();

  for (auto item : d)
  {
    std::string key = item.first.cast<std::string>();
    py::dict propDict = item.second.cast<py::dict>();

    // Use the centralized conversion utility
    auto prop = dictToProperty(propDict);
    pl->SetProperty(key, prop);
  }

  return pl;
}

// Helper function to convert PropertyList to JSON string
std::string propertyListToJsonString(const mitk::PropertyList &pl)
{
  auto dict = convertPropertyListToDict(pl);
  return py::cast<std::string>(py::module::import("json").attr("dumps")(dict));
}

// Helper function to create PropertyList from JSON string
mitk::PropertyList::Pointer propertyListFromJsonString(const std::string &json)
{
  auto dict = py::module::import("json").attr("loads")(json).cast<py::dict>();
  return convertDictToPropertyList(dict);
}

// Helper function for set_property with auto-wrapping
void setPropertyHelper(mitk::PropertyList &pl, const std::string &key, py::object value)
{
  // Use the auto-wrapping helper
  auto prop = resolvePropertyValue(pl, key, value);
  pl.SetProperty(key, prop);
}

/**
 * @brief Initializes Python bindings for MITK PropertyList
 *
 * This function registers the PropertyList class with the Python module,
 * including methods for property management and JSON serialization.
 *
 * @param m The pybind11 module to which the PropertyList bindings should be added
 */
void init_PropertyList(py::module_ &m)
{
  auto propertyList_class = py::class_<mitk::PropertyList, mitk::PropertyList::Pointer>(m, "PropertyList");

  propertyList_class.def(py::init([]() { return mitk::PropertyList::New(); }))
    .def(
      "get_property",
      [](const mitk::PropertyList &pl, const std::string &key) -> mitk::BaseProperty::Pointer
      {
        return pl.GetProperty(key); // returns nullptr -> None if not found
      },
      py::arg("key"),
      py::return_value_policy::reference_internal)
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
      [](mitk::PropertyList &pl, const std::string &key)
      {
        // Use default context and fallback parameters
        pl.RemoveProperty(key, "", false);
      },
      py::arg("key"))
    .def_property_readonly("property_keys",
                           [](const mitk::PropertyList &pl)
                           {
                             auto keys = pl.GetPropertyKeys();
                             return std::vector<std::string>(keys.begin(), keys.end());
                           })
    .def("to_dict", &convertPropertyListToDict)
    .def_static("from_dict", &convertDictToPropertyList)
    .def("to_json", &propertyListToJsonString)
    .def_static("from_json", &propertyListFromJsonString);

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
