/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "Step6RegionGrowing.txx"

#include <mitkInstantiateAccessFunctions.h>

#define InstantiateAccessFunction_RegionGrowing(pixelType, dim) \
template void RegionGrowing(itk::Image<pixelType, dim>*, Step6*);

/// Instantiate the access function for 4D and all datatypes.
/// Instantiation for 3D is done in Step6RegionGrowing2.
/// Some compilers have memory problems without the explicit
/// instantiation, some even need the separation in 2D/3D/4D.
InstantiateAccessFunctionForFixedDimension(RegionGrowing, 4)

/**
\example Step6RegionGrowing3.cpp
*/
