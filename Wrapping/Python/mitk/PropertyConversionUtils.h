/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/**
 * @file PropertyConversionUtils.h
 * @brief Centralized property conversion utilities for Python bindings
 *
 * DESIGN RATIONALE: Header-only implementation
 * -----------------------------------------
 * This file uses a header-only approach for several important reasons:
 *
 * 1. TEMPLATE COMPATIBILITY: These utilities are used across multiple compilation
 *    units with pybind11 templates. Header-only ensures consistent instantiation.
 *
 * 2. RUNTIME ROBUSTNESS: Eliminates potential linking issues and ensures all
 *    compilation units use identical conversion logic.
 *
 * 3. INLINING BENEFITS: Property conversions are performance-sensitive in Python
 *    bindings. Header-only allows compiler inlining optimization.
 *
 * 4. PYTHON BINDINGS LAYER: While MITK typically prefers header/cpp separation,
 *    the Python bindings layer has different constraints and performance profiles.
 *
 * Trade-off: Slightly increased compile times, but this is acceptable for the
 * runtime robustness and maintainability benefits in this specific context.
 */

#pragma once

#include <mitkBaseProperty.h>
#include <mitkColorProperty.h>
#include <mitkProperties.h>
#include <mitkStringProperty.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace mitk
{
  namespace python
  {

    /**
     * @brief Converts a BaseProperty to its Python native value
     *
     * This function converts MITK property objects to their corresponding Python types:
     * - StringProperty -> str
     * - BoolProperty -> bool
     * - IntProperty -> int
     * - FloatProperty -> float
     * - DoubleProperty -> float
     * - ColorProperty -> tuple(float, float, float)
     * - Other types -> returns the property object itself
     *
     * @param prop The property to convert
     * @return Python object representing the property value
     */
    inline py::object propertyToPythonValue(const mitk::BaseProperty &prop)
    {
      if (const mitk::StringProperty *strProp = dynamic_cast<const mitk::StringProperty *>(&prop))
      {
        return py::cast(strProp->GetValue());
      }
      else if (const mitk::BoolProperty *boolProp = dynamic_cast<const mitk::BoolProperty *>(&prop))
      {
        return py::cast(boolProp->GetValue());
      }
      else if (const mitk::IntProperty *intProp = dynamic_cast<const mitk::IntProperty *>(&prop))
      {
        return py::cast(intProp->GetValue());
      }
      else if (const mitk::FloatProperty *floatProp = dynamic_cast<const mitk::FloatProperty *>(&prop))
      {
        return py::cast(floatProp->GetValue());
      }
      else if (const mitk::DoubleProperty *doubleProp = dynamic_cast<const mitk::DoubleProperty *>(&prop))
      {
        return py::cast(doubleProp->GetValue());
      }
      else if (const mitk::ColorProperty *colorProp = dynamic_cast<const mitk::ColorProperty *>(&prop))
      {
        auto color = colorProp->GetColor();
        return py::make_tuple(color[0], color[1], color[2]);
      }

      // For unsupported types, return the property itself
      return py::cast(&prop, py::return_value_policy::reference);
    }

    /**
     * @brief Converts a Python object to a BaseProperty, auto-wrapping if needed
     *
     * This function automatically converts Python objects to appropriate MITK property types:
     * - bool -> BoolProperty
     * - int -> IntProperty
     * - float -> FloatProperty
     * - str -> StringProperty
     * - tuple(float, float, float) -> ColorProperty
     * - BaseProperty subclasses -> used directly
     *
     * @param value The Python object to convert
     * @return MITK BaseProperty pointer
     * @throws py::type_error If the Python type cannot be converted to a property
     */
    inline mitk::BaseProperty::Pointer pythonValueToProperty(py::object value)
    {
      // If already a BaseProperty, use directly
      if (py::isinstance<mitk::BaseProperty>(value))
      {
        return value.cast<mitk::BaseProperty::Pointer>();
      }

      // Auto-wrap by Python type
      // CRITICAL: bool before int (bool is a subclass of int in Python)
      if (py::isinstance<py::bool_>(value))
      {
        return mitk::BoolProperty::New(value.cast<bool>());
      }
      if (py::isinstance<py::int_>(value))
      {
        return mitk::IntProperty::New(value.cast<int>());
      }
      if (py::isinstance<py::float_>(value))
      {
        return mitk::FloatProperty::New(value.cast<float>());
      }
      if (py::isinstance<py::str>(value))
      {
        return mitk::StringProperty::New(value.cast<std::string>());
      }
      if (py::isinstance<py::tuple>(value))
      {
        // Try to convert tuple to ColorProperty (3 floats)
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
                           " to a property type. Pass a mitk.BaseProperty subclass explicitly "
                           "(e.g. mitk.StringProperty(...), mitk.ColorProperty(...)).");
    }

    // Convert a Python object to a BaseProperty, respecting existing type if provided
    inline mitk::BaseProperty::Pointer pythonValueToPropertyWithType(py::object value, const std::string &targetType)
    {
      // If already a BaseProperty, use directly
      if (py::isinstance<mitk::BaseProperty>(value))
      {
        return value.cast<mitk::BaseProperty::Pointer>();
      }

      // If no target type specified, use auto-detection
      if (targetType.empty())
      {
        return pythonValueToProperty(value);
      }

      // Coerce to specific type
      if (targetType == "StringProperty")
      {
        return mitk::StringProperty::New(value.cast<std::string>());
      }
      else if (targetType == "BoolProperty")
      {
        return mitk::BoolProperty::New(value.cast<bool>());
      }
      else if (targetType == "IntProperty")
      {
        return mitk::IntProperty::New(value.cast<int>());
      }
      else if (targetType == "FloatProperty")
      {
        return mitk::FloatProperty::New(value.cast<float>());
      }
      else if (targetType == "DoubleProperty")
      {
        return mitk::DoubleProperty::New(value.cast<double>());
      }
      else if (targetType == "ColorProperty")
      {
        auto valueTuple = value.cast<py::tuple>();
        mitk::Color color;
        color[0] = valueTuple[0].cast<float>();
        color[1] = valueTuple[1].cast<float>();
        color[2] = valueTuple[2].cast<float>();
        return mitk::ColorProperty::New(color);
      }

      throw py::type_error("Cannot auto-convert " + std::string(py::str(value.get_type())) + " to " + targetType +
                           ". Pass the new property value as a mitk." + targetType + " instance explicitly.");
    }

    // Convert BaseProperty to dict for JSON serialization
    inline py::dict propertyToDict(const mitk::BaseProperty &prop)
    {
      py::dict result;
      result["type"] = prop.GetNameOfClass();

      // Add value based on property type
      if (const mitk::StringProperty *strProp = dynamic_cast<const mitk::StringProperty *>(&prop))
      {
        result["value"] = strProp->GetValue();
      }
      else if (const mitk::BoolProperty *boolProp = dynamic_cast<const mitk::BoolProperty *>(&prop))
      {
        result["value"] = boolProp->GetValue();
      }
      else if (const mitk::IntProperty *intProp = dynamic_cast<const mitk::IntProperty *>(&prop))
      {
        result["value"] = intProp->GetValue();
      }
      else if (const mitk::FloatProperty *floatProp = dynamic_cast<const mitk::FloatProperty *>(&prop))
      {
        result["value"] = floatProp->GetValue();
      }
      else if (const mitk::DoubleProperty *doubleProp = dynamic_cast<const mitk::DoubleProperty *>(&prop))
      {
        result["value"] = doubleProp->GetValue();
      }
      else if (const mitk::ColorProperty *colorProp = dynamic_cast<const mitk::ColorProperty *>(&prop))
      {
        auto color = colorProp->GetColor();
        result["value"] = py::make_tuple(color[0], color[1], color[2]);
      }

      return result;
    }

    // Convert dict to BaseProperty for JSON deserialization
    inline mitk::BaseProperty::Pointer dictToProperty(const py::dict &d)
    {
      std::string type = d["type"].cast<std::string>();

      if (type == "StringProperty")
      {
        return mitk::StringProperty::New(d["value"].cast<std::string>());
      }
      else if (type == "BoolProperty")
      {
        return mitk::BoolProperty::New(d["value"].cast<bool>());
      }
      else if (type == "IntProperty")
      {
        return mitk::IntProperty::New(d["value"].cast<int>());
      }
      else if (type == "FloatProperty")
      {
        return mitk::FloatProperty::New(d["value"].cast<float>());
      }
      else if (type == "DoubleProperty")
      {
        return mitk::DoubleProperty::New(d["value"].cast<double>());
      }
      else if (type == "ColorProperty")
      {
        auto value = d["value"].cast<py::tuple>();
        mitk::Color color;
        color[0] = value[0].cast<float>();
        color[1] = value[1].cast<float>();
        color[2] = value[2].cast<float>();
        return mitk::ColorProperty::New(color);
      }

      throw std::runtime_error("Unsupported property type: " + type);
    }

  } // namespace python
} // namespace mitk
