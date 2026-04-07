/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkBaseProperty.h>
#include <mitkPropertyList.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

// Note: The actual serialization functions are already implemented in Property.cpp and PropertyList.cpp
// This file is a placeholder for any additional serialization-related functionality
// that might be needed beyond what's already in those files.

// The main serialization functions are:
// - convertPropertyToDict() in Property.cpp
// - convertDictToProperty() in Property.cpp
// - convertPropertyListToDict() in PropertyList.cpp
// - convertDictToPropertyList() in PropertyList.cpp
// - propertyListToJsonString() in PropertyList.cpp
// - propertyListFromJsonString() in PropertyList.cpp

// This file can be extended if additional serialization features are needed
// For example, custom serialization for specific property types

/**
 * @brief Initializes Python bindings for property serialization utilities
 *
 * This function can be used to register any additional serialization-related
 * functionality with the Python module. Currently, all main serialization
 * functions are already registered in their respective files.
 *
 * @param m The pybind11 module to which serialization bindings should be added
 */
void init_PropertySerialization(py::module &m)
{
  // This function can be used to register any additional serialization-related
  // functionality with the Python module

  // For now, it's empty since all the main serialization functions are
  // already registered in their respective files
}
