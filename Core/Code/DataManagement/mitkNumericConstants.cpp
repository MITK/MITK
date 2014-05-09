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

#include "mitkNumericConstants.h"
#include "vnl/vnl_math.h"
#include <limits>

const mitk::ScalarType mitk::eps     = vnl_math::eps*100;
const mitk::ScalarType mitk::sqrteps = vnl_math::sqrteps;
extern const mitk::ScalarType mitk::large   = std::numeric_limits<mitk::ScalarType>::max();
