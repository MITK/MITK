/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef PropertyOwnerBindings_h
#define PropertyOwnerBindings_h

#include "PropertyAutoWrap.h"
#include "PropertyConversionUtils.h"
#include "PropertyNotOwnedError.h"
#include <mitkIPropertyOwner.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

/**
 * \brief Binds IPropertyOwner methods to a Python class.
 *
 * Adds get_property / set_property / remove_property / property_is_owned /
 * property_keys to the given pybind11 class. The underlying C++ type is
 * inferred from \c PyClass::type, so call sites stay terse:
 *
 * \code
 * bind_property_owner(image_class);
 * \endcode
 */
template <typename PyClass>
void bind_property_owner(PyClass &cls)
{
  using CppClass = typename PyClass::type;

  cls.def(
    "get_property",
    [](const CppClass &obj, const std::string &key, bool raw) -> py::object
    {
      auto prop = obj.GetConstProperty(key);
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
    "Pass ``raw=True`` to always get the underlying ``mitk.BaseProperty`` object\n"
    "(e.g. to access metadata or pass it to a C++ function that expects one).");

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
        throw PropertyNotOwnedError("Property '" + key +
                                    "' is provided read-only by this object "
                                    "and cannot be changed via set_property(). "
                                    "It may be owned by an internal component "
                                    "(e.g., a Label in a MultiLabelSegmentation).");
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
        throw PropertyNotOwnedError("Property '" + key +
                                    "' is provided read-only by this object "
                                    "and cannot be removed via remove_property().");
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

#endif
