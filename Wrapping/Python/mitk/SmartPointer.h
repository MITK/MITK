/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef SmartPointer_h
#define SmartPointer_h

#include <pybind11/pybind11.h>
#include <itkSmartPointer.h>

PYBIND11_DECLARE_HOLDER_TYPE(T, itk::SmartPointer<T>, true);

#endif
