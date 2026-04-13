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
      [](const mitk::PropertyList &pl, const std::string &key, bool raw) -> py::object
      {
        auto prop = pl.GetProperty(key);
        if (!prop)
          return py::none();
        // raw=True preserves the old behaviour and returns the mitk.BaseProperty object.
        if (raw)
          return py::cast(prop, py::return_value_policy::reference);
        return mitk::python::propertyToPythonValue(*prop);
      },
      py::arg("key"),
      py::arg("raw") = false,
      "Return the property value for *key*, or None if not set.\n\n"
      "By default returns a coerced Python-native value: ``bool``, ``int``, ``float``,\n"
      "``str``, or an ``(r, g, b)`` tuple for ColorProperty. For types without a known\n"
      "Python equivalent the raw ``mitk.BaseProperty`` object is returned.\n\n"
      "Pass ``raw=True`` to always get the underlying ``mitk.BaseProperty`` object.")
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
