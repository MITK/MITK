/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "Step6RegionGrowing.txx"

#include <mitkInstantiateAccessFunctions.h>

#define InstantiateAccessFunction_RegionGrowing(pixelType, dim)                                                        \
  \
template void                                                                                                          \
    RegionGrowing(itk::Image<pixelType, dim> *, Step6 *);

/// Instantiate the access function for 2D and all datatypes.
/// Instantiation for 3D is done in Step6RegionGrowing2.
/// Some compilers have memory problems without the explicit
/// instantiation, some even need the separation in 2D/3D.
InstantiateAccessFunctionForFixedDimension(RegionGrowing, 2)

  /**
  \example Step6RegionGrowing1.cpp
  */
