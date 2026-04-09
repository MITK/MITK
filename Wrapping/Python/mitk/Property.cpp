/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "PropertyConversionUtils.h"
#include "SmartPointer.h"
#include "TemporoSpatialStringSerialization.h"
#include <Python.h>
#include <mitkBaseProperty.h>
#include <mitkColorProperty.h>
#include <mitkProperties.h>
#include <mitkStringProperty.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

// Use the centralized conversion utilities
using mitk::python::dictToProperty;
using mitk::python::propertyToDict;

/**
 * @brief Initializes Python bindings for MITK property types
 *
 * This function registers all MITK property types with the Python module,
 * including BaseProperty and its concrete subclasses (StringProperty, BoolProperty, etc.).
 *
 * @param m The pybind11 module to which the property bindings should be added
 */
void init_Property(py::module_ &m)
{
  // Create PropertyNotOwnedError exception using Python C API
  // This approach works with pybind11 v3.0.1
  PyObject *exc = PyErr_NewException("mitk.PropertyNotOwnedError", PyExc_AttributeError, nullptr);
  if (exc == nullptr)
  {
    PyErr_Clear();
    return;
  }

  // Set the docstring
  PyObject *doc = PyUnicode_FromString("Raised when set_property() or remove_property() is called on a property\n"
                                       "that is provided read-only (not owned) by this object.\n\n"
                                       "Use property_is_owned(key) to check before writing.");
  PyObject_SetAttrString(exc, "__doc__", doc);
  Py_DECREF(doc);

  // Add to module
  m.add_object("PropertyNotOwnedError", py::reinterpret_borrow<py::object>(exc));

  // BaseProperty binding - using the same pattern as Image.cpp
  py::class_<mitk::BaseProperty, mitk::BaseProperty::Pointer>(m, "BaseProperty")
    .def("__str__", &mitk::BaseProperty::GetValueAsString)
    .def("__repr__",
         [](const mitk::BaseProperty &p)
         { return "<mitk." + std::string(p.GetNameOfClass()) + ": " + p.GetValueAsString() + ">"; })
    .def("__eq__", [](const mitk::BaseProperty &a, const mitk::BaseProperty &b) { return a == b; })
    .def_property_readonly("value", nullptr) // Overridden by subclasses
    .def("to_dict",
         [](const mitk::BaseProperty& p) {
           py::dict result;
           if (mitk::python::tryTemporoSpatialStringToDict(p, result)) return result;
           return propertyToDict(p);
         })
    .def("clone", [](const mitk::BaseProperty &p) { return p.Clone(); });

  // StringProperty binding - use lambda for protected constructor
  py::class_<mitk::StringProperty, mitk::BaseProperty, mitk::StringProperty::Pointer>(m, "StringProperty")
    .def(py::init([](const std::string &value) { return mitk::StringProperty::New(value); }), py::arg("value") = "")
    .def_property_readonly("value", &mitk::StringProperty::GetValue);

  // BoolProperty binding - use lambda for protected constructor
  py::class_<mitk::BoolProperty, mitk::BaseProperty, mitk::BoolProperty::Pointer>(m, "BoolProperty")
    .def(py::init([](bool value) { return mitk::BoolProperty::New(value); }), py::arg("value") = false)
    .def_property_readonly("value", &mitk::BoolProperty::GetValue);

  // IntProperty binding - use lambda for protected constructor
  py::class_<mitk::IntProperty, mitk::BaseProperty, mitk::IntProperty::Pointer>(m, "IntProperty")
    .def(py::init([](int value) { return mitk::IntProperty::New(value); }), py::arg("value") = 0)
    .def_property_readonly("value", &mitk::IntProperty::GetValue);

  // FloatProperty binding - use lambda for protected constructor
  py::class_<mitk::FloatProperty, mitk::BaseProperty, mitk::FloatProperty::Pointer>(m, "FloatProperty")
    .def(py::init([](float value) { return mitk::FloatProperty::New(value); }), py::arg("value") = 0.0f)
    .def_property_readonly("value", &mitk::FloatProperty::GetValue);

  // DoubleProperty binding - use lambda for protected constructor
  py::class_<mitk::DoubleProperty, mitk::BaseProperty, mitk::DoubleProperty::Pointer>(m, "DoubleProperty")
    .def(py::init([](double value) { return mitk::DoubleProperty::New(value); }), py::arg("value") = 0.0)
    .def_property_readonly("value", &mitk::DoubleProperty::GetValue);

  // ColorProperty binding - use lambda for protected constructor
  py::class_<mitk::ColorProperty, mitk::BaseProperty, mitk::ColorProperty::Pointer>(m, "ColorProperty")
    .def(py::init([](const mitk::Color &value) { return mitk::ColorProperty::New(value); }), py::arg("value"))
    .def_static("from_rgb",
                [](float r, float g, float b)
                {
                  mitk::Color color;
                  color[0] = r;
                  color[1] = g;
                  color[2] = b;
                  return mitk::ColorProperty::New(color);
                })
    .def_property_readonly("value",
                           [](const mitk::ColorProperty &p)
                           {
                             auto color = p.GetColor();
                             return py::make_tuple(color[0], color[1], color[2]);
                           });

  // Module-level factory function
  m.def("property_from_dict",
        [](const py::dict& d) -> mitk::BaseProperty::Pointer {
          if (auto ts = mitk::python::tryDictToTemporoSpatialString(d)) return ts;
          return dictToProperty(d);
        },
        py::arg("d"),
        "Reconstruct a BaseProperty from a dict produced by to_dict().");
}
