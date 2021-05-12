/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAffineTransform3D_h
#define mitkAffineTransform3D_h

#include <mitkNumericConstants.h>
#include <itkScalableAffineTransform.h>

namespace mitk
{
  using AffineTransform3D = itk::ScalableAffineTransform<ScalarType, 3>;
}

#endif
