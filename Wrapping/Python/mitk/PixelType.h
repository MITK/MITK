/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef PixelType_h
#define PixelType_h

#include <mitkPixelType.h>
#include <pybind11/pybind11.h>

namespace mitk
{
  PixelType MakePixelType(const pybind11::object& dtype, size_t components = 1);
}

#endif
