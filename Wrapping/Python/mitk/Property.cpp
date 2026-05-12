/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "PropertyConversionUtils.h"
#include "PropertyNotOwnedError.h"
#include "SmartPointer.h"
#include "TemporoSpatialStringSerialization.h"
#include <Python.h>
#include <mitkBaseProperty.h>
#include <mitkColorProperty.h>
#include <mitkProperties.h>
#include <mitkStringProperty.h>
#include <nlohmann/json.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

/**
 * \brief Initializes Python bindings for MITK property types.
 *
 * Registers BaseProperty and its concrete subclasses (StringProperty,
 * BoolProperty, IntProperty, FloatProperty, DoubleProperty, ColorProperty),
 * plus the PropertyNotOwnedError exception.
 */
void InitProperty(py::module_ &m)
{
  auto propertyNotOwnedError =
    py::register_exception<PropertyNotOwnedError>(m, "PropertyNotOwnedError", PyExc_AttributeError);
  propertyNotOwnedError.attr("__doc__") =
    R"(Raised when ``set_property()`` or ``remove_property()`` is called on a
property that is provided read-only (not owned) by this object.

Subclass of ``AttributeError``. Use :py:meth:`property_is_owned` to check
before writing.
)";

  py::class_<mitk::BaseProperty, mitk::BaseProperty::Pointer>(m, "BaseProperty",
    R"(Abstract base class for typed property values.

Every property attached to an MITK object is a ``BaseProperty`` subclass:
:py:class:`StringProperty`, :py:class:`BoolProperty`,
:py:class:`IntProperty`, :py:class:`FloatProperty`,
:py:class:`DoubleProperty`, :py:class:`ColorProperty`,
:py:class:`TemporoSpatialStringProperty`.

When you read properties through the :py:attr:`Image.properties` mapping
(or via :py:meth:`get_property`), values are auto-coerced to Python-native
types. The raw ``BaseProperty`` is reachable with ``raw=True``.
)")
    .def("__str__", &mitk::BaseProperty::GetValueAsString,
      "Return the property value as a string.")
    .def("__repr__",
         [](const mitk::BaseProperty &p)
         { return "<mitk." + std::string(p.GetNameOfClass()) + ": " + p.GetValueAsString() + ">"; })
    .def("__eq__", [](const mitk::BaseProperty &a, const mitk::BaseProperty &b) { return a == b; })
    .def("to_json",
         [](const mitk::BaseProperty &p) { return mitk::ConvertPropertyToSelfContainedJson(&p).dump(); },
         R"(Serialize this property to a self-contained JSON string.

The string carries enough type information that :py:meth:`from_json` can
reconstruct the correct subclass.

Returns:
    JSON string representation of the property.
)")
    .def_static(
      "from_json",
      [](const std::string &json) { return mitk::ConvertPropertyFromSelfContainedJson(nlohmann::json::parse(json)); },
      py::arg("json"),
      R"(Reconstruct a ``BaseProperty`` subclass from JSON.

Args:
    json: JSON string produced by :py:meth:`to_json`.

Returns:
    A new ``BaseProperty`` of the appropriate subclass.
)")
    .def_property_readonly("value", nullptr,
      "Property value (overridden by subclasses with type-specific accessors).")
    .def("clone", [](const mitk::BaseProperty &p) { return p.Clone(); },
      "Return a deep copy of this property.");

  py::class_<mitk::StringProperty, mitk::BaseProperty, mitk::StringProperty::Pointer>(m, "StringProperty",
    "Property holding a single ``str`` value.")
    .def(py::init([](const std::string &value) { return mitk::StringProperty::New(value); }), py::arg("value") = "",
      "Construct a string property with the given value (default empty).")
    .def_property_readonly("value", &mitk::StringProperty::GetValue,
      "The stored string.");

  py::class_<mitk::BoolProperty, mitk::BaseProperty, mitk::BoolProperty::Pointer>(m, "BoolProperty",
    "Property holding a single ``bool`` value.")
    .def(py::init([](bool value) { return mitk::BoolProperty::New(value); }), py::arg("value") = false,
      "Construct a bool property with the given value (default ``False``).")
    .def_property_readonly("value", &mitk::BoolProperty::GetValue,
      "The stored bool.");

  py::class_<mitk::IntProperty, mitk::BaseProperty, mitk::IntProperty::Pointer>(m, "IntProperty",
    "Property holding a single ``int`` value.")
    .def(py::init([](int value) { return mitk::IntProperty::New(value); }), py::arg("value") = 0,
      "Construct an int property with the given value (default 0).")
    .def_property_readonly("value", &mitk::IntProperty::GetValue,
      "The stored int.");

  py::class_<mitk::FloatProperty, mitk::BaseProperty, mitk::FloatProperty::Pointer>(m, "FloatProperty",
    "Property holding a single ``float`` (single-precision) value.")
    .def(py::init([](float value) { return mitk::FloatProperty::New(value); }), py::arg("value") = 0.0f,
      "Construct a float property with the given value (default 0.0).")
    .def_property_readonly("value", &mitk::FloatProperty::GetValue,
      "The stored float.");

  py::class_<mitk::DoubleProperty, mitk::BaseProperty, mitk::DoubleProperty::Pointer>(m, "DoubleProperty",
    "Property holding a single ``float`` (double-precision) value.")
    .def(py::init([](double value) { return mitk::DoubleProperty::New(value); }), py::arg("value") = 0.0,
      "Construct a double property with the given value (default 0.0).")
    .def_property_readonly("value", &mitk::DoubleProperty::GetValue,
      "The stored double.");

  py::class_<mitk::ColorProperty, mitk::BaseProperty, mitk::ColorProperty::Pointer>(m, "ColorProperty",
    R"(Property holding an RGB color.

When read through the high-level property API the value is coerced to an
``(r, g, b)`` tuple of floats in ``[0, 1]``.
)")
    .def(py::init([](const mitk::Color &value) { return mitk::ColorProperty::New(value); }), py::arg("value"),
      R"(Construct a color property from a :py:class:`Color`.

Args:
    value: The color value.
)")
    .def_static("from_rgb",
                [](float r, float g, float b)
                {
                  mitk::Color color;
                  color[0] = r;
                  color[1] = g;
                  color[2] = b;
                  return mitk::ColorProperty::New(color);
                },
                R"(Construct a color property from three floats.

Args:
    r: Red component in ``[0, 1]``.
    g: Green component in ``[0, 1]``.
    b: Blue component in ``[0, 1]``.

Returns:
    A new :py:class:`ColorProperty`.
)")
    .def_property_readonly("value", &mitk::ColorProperty::GetColor,
      "The stored :py:class:`Color`.");

  m.def(
    "property_from_dict",
    [](const py::dict &d) -> mitk::BaseProperty::Pointer
    {
      if (d.contains("type") &&
          d["type"].cast<std::string>() == "TemporoSpatialStringProperty")
        return mitk::python::tryDictToTemporoSpatialString(d);
      return mitk::python::dictToProperty(d);
    },
    py::arg("d"),
    R"(Reconstruct a ``BaseProperty`` subclass from a dict.

Consumes the dict layout used by :py:meth:`PropertyList.to_dict` (per-entry
values) and by :py:meth:`TemporoSpatialStringProperty.to_dict`. Inspects
the ``"type"`` key in the dict to pick the correct subclass.

Args:
    d: Dict describing a single property's type and value.

Returns:
    A new ``BaseProperty`` of the appropriate subclass.
)");
}
