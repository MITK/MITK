/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkScaleOperation.h"

mitk::ScaleOperation::ScaleOperation(mitk::OperationType operationType,
                                     mitk::Point3D scaleFactor,
                                     mitk::Point3D scaleAnchorPoint)
  : mitk::Operation(operationType), m_ScaleFactor(scaleFactor), m_ScaleAnchorPoint(scaleAnchorPoint)
{
}

mitk::ScaleOperation::~ScaleOperation()
{
}

mitk::Point3D mitk::ScaleOperation::GetScaleFactor()
{
  return m_ScaleFactor;
}

mitk::Point3D mitk::ScaleOperation::GetScaleAnchorPoint()
{
  return m_ScaleAnchorPoint;
}
