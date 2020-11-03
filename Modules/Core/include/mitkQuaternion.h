/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKQUATERNION_H_
#define MITKQUATERNION_H_

#include "mitkNumericConstants.h"
#include <vnl/vnl_quaternion.h>

namespace mitk
{
  typedef vnl_quaternion<ScalarType> Quaternion;
}

#endif /* MITKQUATERNION_H_ */
