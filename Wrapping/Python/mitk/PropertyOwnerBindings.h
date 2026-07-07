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
      if (raw)
        return py::cast(prop, py::return_value_policy::reference);
      return mitk::python::propertyToPythonValue(*prop);
    },
    py::arg("key"),
    py::arg("raw") = false,
    R"(Return the property value for *key*, or ``None`` if not set.

By default returns a coerced Python-native value: ``bool``, ``int``,
``float``, ``str``, or an ``(r, g, b)`` tuple for ``ColorProperty``. For
types without a known Python equivalent the raw ``mitk.BaseProperty``
object is returned.

Args:
    key: Property name.
    raw: If True, always return the underlying ``mitk.BaseProperty`` object
        (useful to access metadata or to pass to a C++ function that
        expects one). Defaults to False.

Returns:
    The property value, or ``None`` if the property is not set.
)");

  cls.def(
    "property_is_owned",
    [](const CppClass &obj, const std::string &key) -> bool { return obj.PropertyIsOwned(key); },
    py::arg("key"),
    R"(Return True if the property *key* is owned (writable) by this object.

Returns False if the property does not exist or is provided read-only
(for example, routed from an internal component like a Label inside a
MultiLabelSegmentation).

Args:
    key: Property name.
)");

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
    py::arg("value"),
    R"(Set the property *key* to *value*.

The value is auto-wrapped into the appropriate ``BaseProperty`` subtype
based on its Python type: ``bool`` -> ``BoolProperty``, ``int`` ->
``IntProperty``, ``float`` -> ``DoubleProperty``, ``str`` ->
``StringProperty``, 3-tuple -> ``ColorProperty``, or an existing
``mitk.BaseProperty`` instance is used as-is.

Args:
    key: Property name.
    value: Property value (auto-wrapped to a ``BaseProperty`` subtype).

Raises:
    PropertyNotOwnedError: If the property is provided read-only (not
        owned) by this object.
)");

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
    py::arg("key"),
    R"(Remove the property *key* if it exists.

Args:
    key: Property name.

Raises:
    PropertyNotOwnedError: If the property is provided read-only and
        cannot be removed.
)");

  cls.def_property_readonly("property_keys",
                            [](const CppClass &obj)
                            {
                              auto keys = obj.GetPropertyKeys();
                              return std::vector<std::string>(keys.begin(), keys.end());
                            },
                            "List of all property keys currently set on this object.");
}

#endif
