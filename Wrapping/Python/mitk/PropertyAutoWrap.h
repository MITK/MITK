/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#pragma once

#include "PropertyConversionUtils.h"
#include <mitkBaseProperty.h>
#include <mitkIPropertyOwner.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

// Use the centralized conversion utilities
using mitk::python::pythonValueToProperty;
using mitk::python::pythonValueToPropertyWithType;

// Helper function to coerce value to existing property type
inline mitk::BaseProperty::Pointer coerceToExistingType(const mitk::BaseProperty *existing, py::object value)
{
  if (nullptr == existing)
  {
    throw std::runtime_error("Cannot coerce to existing type: no existing property");
  }

  // Get the type of existing property
  std::string existingType = existing->GetNameOfClass();

  // Coerce to the existing type
  return pythonValueToPropertyWithType(value, existingType);
}

// Main function to resolve property value (auto-wrap or use existing type)
inline mitk::BaseProperty::Pointer resolvePropertyValue(mitk::IPropertyOwner &owner,
                                                        const std::string &key,
                                                        py::object value)
{
  // If already a BaseProperty, use directly
  if (py::isinstance<mitk::BaseProperty>(value))
  {
    return value.cast<mitk::BaseProperty::Pointer>();
  }

  // Check if key already has a property -- respect its type
  auto existing = owner.GetConstProperty(key);
  if (existing)
  {
    return coerceToExistingType(existing, value);
  }

  // Auto-wrap by Python type
  return pythonValueToProperty(value);
}
