/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef PropertyAutoWrap_h
#define PropertyAutoWrap_h

#include <mitkBaseProperty.h>
#include <mitkColorProperty.h>
#include <mitkIPropertyOwner.h>
#include <mitkProperties.h>
#include <mitkStringProperty.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

/**
 * \brief Converts a Python object to a BaseProperty, auto-wrapping common scalar types.
 *
 * Supported auto-wrap conversions:
 * - bool   -> BoolProperty
 * - int    -> IntProperty
 * - float  -> FloatProperty
 * - str    -> StringProperty
 * - tuple(float, float, float) -> ColorProperty
 * - BaseProperty subclasses -> used directly
 *
 * \throws py::type_error if the Python type cannot be auto-wrapped.
 */
inline mitk::BaseProperty::Pointer pythonValueToProperty(py::object value)
{
  if (py::isinstance<mitk::BaseProperty>(value))
    return value.cast<mitk::BaseProperty::Pointer>();

  // bool before int: bool is a subclass of int in Python.
  if (py::isinstance<py::bool_>(value))
    return mitk::BoolProperty::New(value.cast<bool>());

  if (py::isinstance<py::int_>(value))
    return mitk::IntProperty::New(value.cast<int>());

  if (py::isinstance<py::float_>(value))
    return mitk::FloatProperty::New(value.cast<float>());

  if (py::isinstance<py::str>(value))
    return mitk::StringProperty::New(value.cast<std::string>());

  if (py::isinstance<py::tuple>(value))
  {
    auto tupleValue = value.cast<py::tuple>();
    if (tupleValue.size() == 3)
    {
      mitk::Color color;
      color[0] = tupleValue[0].cast<float>();
      color[1] = tupleValue[1].cast<float>();
      color[2] = tupleValue[2].cast<float>();
      return mitk::ColorProperty::New(color);
    }
  }

  throw py::type_error("Cannot auto-convert " + std::string(py::str(value.get_type())) +
                       " to a property. Pass a mitk.BaseProperty subclass explicitly "
                       "(e.g. mitk.StringProperty(...), mitk.ColorProperty(...)).");
}

/**
 * \brief Converts a Python object to a BaseProperty, coercing to a specific target class when given.
 *
 * If \c targetType is empty, falls back to pythonValueToProperty() (type inference).
 * Otherwise, coerces the Python value to the named property class.
 */
inline mitk::BaseProperty::Pointer pythonValueToPropertyWithType(py::object value, const std::string &targetType)
{
  if (py::isinstance<mitk::BaseProperty>(value))
    return value.cast<mitk::BaseProperty::Pointer>();

  if (targetType.empty())
    return pythonValueToProperty(value);

  if (targetType == "StringProperty")
    return mitk::StringProperty::New(value.cast<std::string>());

  if (targetType == "BoolProperty")
    return mitk::BoolProperty::New(value.cast<bool>());

  if (targetType == "IntProperty")
    return mitk::IntProperty::New(value.cast<int>());

  if (targetType == "FloatProperty")
    return mitk::FloatProperty::New(value.cast<float>());

  if (targetType == "DoubleProperty")
    return mitk::DoubleProperty::New(value.cast<double>());

  if (targetType == "ColorProperty")
  {
    auto valueTuple = value.cast<py::tuple>();
    mitk::Color color;
    color[0] = valueTuple[0].cast<float>();
    color[1] = valueTuple[1].cast<float>();
    color[2] = valueTuple[2].cast<float>();
    return mitk::ColorProperty::New(color);
  }

  throw py::type_error("Cannot coerce " + std::string(py::str(value.get_type())) + " to " + targetType +
                       ". Pass the new property value as a mitk." + targetType + " instance explicitly.");
}

/**
 * \brief Coerces a Python value to the class of an existing property.
 */
inline mitk::BaseProperty::Pointer coerceToExistingType(const mitk::BaseProperty *existing, py::object value)
{
  if (nullptr == existing)
    throw std::runtime_error("Cannot coerce to existing type: no existing property");

  return pythonValueToPropertyWithType(value, existing->GetNameOfClass());
}

/**
 * \brief Resolves a Python value to a BaseProperty, respecting any existing property's type.
 *
 * Resolution order:
 * 1. If the value is already a BaseProperty, use it directly.
 * 2. If \c key already has a property on \c owner, coerce to that type.
 * 3. Otherwise auto-wrap by Python type.
 */
inline mitk::BaseProperty::Pointer resolvePropertyValue(mitk::IPropertyOwner &owner,
                                                        const std::string &key,
                                                        py::object value)
{
  if (py::isinstance<mitk::BaseProperty>(value))
    return value.cast<mitk::BaseProperty::Pointer>();

  auto existing = owner.GetConstProperty(key);
  if (existing)
    return coerceToExistingType(existing, value);

  return pythonValueToProperty(value);
}

#endif
