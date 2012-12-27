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

#include "mitkVector.h"
#include <vtkSystemIncludes.h>
#include <vtkMatrix4x4.h>

const mitk::ScalarType mitk::eps     = vnl_math::float_eps*100;
const mitk::ScalarType mitk::sqrteps = vnl_math::float_sqrteps;
extern const double mitk::large      = VTK_LARGE_FLOAT;
