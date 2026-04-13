/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/**
 * @file PropertyOwnerBindings.h
 * @brief Reusable template for binding IPropertyOwner methods to Python
 */

#pragma once

#include "PropertyAutoWrap.h"
#include <mitkIPropertyOwner.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

/**
 * @brief Binds IPropertyOwner methods to a Python class
 *
 * This template function adds property-related methods to a Python class that wraps
 * an MITK class implementing IPropertyOwner. It provides methods for getting properties
 * and checking property ownership.
 *
 * @tparam PyClass The pybind11 class type
 * @tparam CppClass The C++ class type that implements IPropertyOwner
 * @param cls The pybind11 class to extend with property owner methods
 */
template <typename PyClass, typename CppClass>
void bind_property_owner(PyClass &cls)
{
  cls.def(
    "get_property",
    [](const CppClass &obj, const std::string &key) -> mitk::BaseProperty::ConstPointer
    {
      return obj.GetConstProperty(key); // None if not found
    },
    py::arg("key"));

  cls.def(
    "property_is_owned",
    [](const CppClass &obj, const std::string &key) -> bool { return obj.PropertyIsOwned(key); },
    py::arg("key"),
    "Return True if the property *key* is owned (writable) by this object.\n\n"
    "Returns False if the property does not exist or is provided read-only\n"
    "(e.g., routed from an internal component like a Label).");

  cls.def(
    "set_property",
    [](CppClass &obj, const std::string &key, py::object value)
    {
      auto existing = obj.GetConstProperty(key);
      if (existing && !obj.PropertyIsOwned(key))
      {
        const std::string message = "Property '" + key +
                                    "' is provided read-only by this object "
                                    "and cannot be changed via set_property(). "
                                    "It may be owned by an internal component "
                                    "(e.g., a Label in a MultiLabelSegmentation).";
        py::object exc_type = py::module_::import("mitk").attr("PropertyNotOwnedError");
        PyErr_SetObject(exc_type.ptr(), exc_type(message).ptr());
        throw py::error_already_set();
      }
      obj.SetProperty(key, resolvePropertyValue(obj, key, value));
    },
    py::arg("key"),
    py::arg("value"));

  cls.def(
    "remove_property",
    [](CppClass &obj, const std::string &key)
    {
      auto existing = obj.GetConstProperty(key);
      if (existing && !obj.PropertyIsOwned(key))
      {
        const std::string message = "Property '" + key +
                                    "' is provided read-only by this object "
                                    "and cannot be removed via remove_property().";
        py::object exc_type = py::module_::import("mitk").attr("PropertyNotOwnedError");
        PyErr_SetObject(exc_type.ptr(), exc_type(message).ptr());
        throw py::error_already_set();
      }
      obj.RemoveProperty(key);
    },
    py::arg("key"));

  cls.def_property_readonly("property_keys",
                            [](const CppClass &obj)
                            {
                              auto keys = obj.GetPropertyKeys();
                              return std::vector<std::string>(keys.begin(), keys.end());
                            });
}
