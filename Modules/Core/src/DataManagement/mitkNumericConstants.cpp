/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkNumericConstants.h"
#include "vnl/vnl_math.h"
#include <limits>

const mitk::ScalarType mitk::eps = vnl_math::eps * 100;
const mitk::ScalarType mitk::sqrteps = vnl_math::sqrteps;
extern const mitk::ScalarType mitk::large = std::numeric_limits<mitk::ScalarType>::max();
