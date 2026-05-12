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
#include "TemporoSpatialStringSerialization.h"
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
  auto propertyList_class = py::class_<mitk::PropertyList, mitk::PropertyList::Pointer>(m, "PropertyList",
    R"(Standalone, owning collection of typed properties.

A ``PropertyList`` is a dict-like structure that maps ``str`` keys to
:py:class:`BaseProperty` subclass values. Unlike the property view on an
:py:class:`Image`, every property in a ``PropertyList`` is owned (writable).

The :py:attr:`properties` accessor returns a
:py:class:`mitk.property_view.PropertyView` for ergonomic dict-style use.
)");

  propertyList_class.def(py::init([]() { return mitk::PropertyList::New(); }),
      "Construct an empty property list.")
    .def(
      "get_property",
      [](const mitk::PropertyList &pl, const std::string &key, bool raw) -> py::object
      {
        auto prop = pl.GetProperty(key);
        if (!prop)
          return py::none();
        if (raw)
          return py::cast(prop, py::return_value_policy::reference);
        return mitk::python::propertyToPythonValue(*prop);
      },
      py::arg("key"),
      py::arg("raw") = false,
      R"(Return the property value for *key*, or ``None`` if not set.

By default returns a coerced Python-native value (``bool``, ``int``,
``float``, ``str``, or ``(r, g, b)`` tuple for color properties). Pass
``raw=True`` to always get the underlying ``mitk.BaseProperty`` object.

Args:
    key: Property name.
    raw: Return the underlying property object instead of a coerced
        Python value. Defaults to False.
)")
    .def(
      "property_is_owned",
      [](const mitk::PropertyList &pl, const std::string &key)
      {
        return pl.GetProperty(key) != nullptr;
      },
      py::arg("key"),
      R"(Return True if *key* exists in the list.

``PropertyList`` owns every property it stores, so this is equivalent to
``key in pl.properties``.
)")
    .def("set_property", &setPropertyHelper, py::arg("key"), py::arg("value"),
      R"(Set the property *key* to *value*.

The value is auto-wrapped into the appropriate ``BaseProperty`` subtype
(see :py:meth:`mitk.Image.set_property` for the type mapping).
)")
    .def(
      "remove_property",
      [](mitk::PropertyList &pl, const std::string &key) { pl.RemoveProperty(key); },
      py::arg("key"),
      "Remove the property *key* if it exists.")
    .def_property_readonly("property_keys",
                           [](const mitk::PropertyList &pl)
                           {
                             auto keys = pl.GetPropertyKeys();
                             return std::vector<std::string>(keys.begin(), keys.end());
                           },
                           "List of all property keys currently stored.")
    .def("to_json",
         [](const mitk::PropertyList &pl) { return mitk::ConvertPropertyListToSelfContainedJson(&pl).dump(); },
         R"(Serialize this property list to a self-contained JSON string.

The string carries type information for every property so the list can be
fully reconstructed with :py:meth:`from_json`.
)")
    .def_static(
      "from_json",
      [](const std::string &json)
      { return mitk::ConvertPropertyListFromSelfContainedJson(nlohmann::json::parse(json)); },
      py::arg("json"),
      R"(Reconstruct a property list from JSON.

Args:
    json: JSON string produced by :py:meth:`to_json`.
)")
    .def(
      "to_dict",
      [](const mitk::PropertyList &pl)
      {
        py::dict result;
        for (const auto &key : pl.GetPropertyKeys())
        {
          const auto *prop = pl.GetProperty(key);
          py::dict propDict;
          if (!mitk::python::tryTemporoSpatialStringToDict(*prop, propDict))
            propDict = mitk::python::propertyToDict(*prop);
          result[py::str(key)] = propDict;
        }
        return result;
      },
      R"(Serialize all properties to a Python ``dict`` keyed by property name.

Each value is itself a dict describing the property's type and content,
matching the format consumed by :py:func:`property_from_dict`.
)")
    .def_static(
      "from_dict",
      [](const py::dict &d)
      {
        auto pl = mitk::PropertyList::New();
        for (const auto &item : d)
        {
          const std::string key = item.first.cast<std::string>();
          const py::dict propDict = item.second.cast<py::dict>();
          mitk::BaseProperty::Pointer prop;
          if (propDict.contains("type") &&
              propDict["type"].cast<std::string>() == "TemporoSpatialStringProperty")
            prop = mitk::python::tryDictToTemporoSpatialString(propDict);
          else
            prop = mitk::python::dictToProperty(propDict);
          if (prop)
            pl->SetProperty(key, prop);
        }
        return pl;
      },
      py::arg("d"),
      R"(Reconstruct a :py:class:`PropertyList` from a dict produced by :py:meth:`to_dict`.

Args:
    d: Dict mapping property names to property dicts.

Returns:
    A new :py:class:`PropertyList` with the properties from *d*.
)");

  propertyList_class.def_property_readonly(
    "properties",
    [](mitk::PropertyList &self)
    {
      py::module_ propertyViewModule = py::module_::import("mitk.property_view");
      py::object PropertyView = propertyViewModule.attr("PropertyView");
      return PropertyView(self);
    },
    py::return_value_policy::reference,
    R"(Live, mutable view of the list's properties.

Returns a :py:class:`mitk.property_view.PropertyView`, a ``MutableMapping``
suitable for dict-style access.
)");
}
