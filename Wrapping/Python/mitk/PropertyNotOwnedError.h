/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef PropertyNotOwnedError_h
#define PropertyNotOwnedError_h

#include <stdexcept>

/**
 * \brief Thrown when a write operation targets a property that is provided
 *        read-only (not owned) by the surrounding object.
 *
 * Registered with pybind11 in Property.cpp so that it surfaces on the Python
 * side as \c mitk.PropertyNotOwnedError (a subclass of \c AttributeError).
 */
class PropertyNotOwnedError : public std::runtime_error
{
public:
  using std::runtime_error::runtime_error;
};

#endif
