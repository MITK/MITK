/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRotationOperation.h"

mitk::RotationOperation::RotationOperation(OperationType operationType,
                                           Point3D pointOfRotation,
                                           Vector3D vectorOfRotation,
                                           ScalarType angleOfRotation)
  : mitk::Operation(operationType),
    m_AngleOfRotation(angleOfRotation),
    m_PointOfRotation(pointOfRotation),
    m_VectorOfRotation(vectorOfRotation)
{
}

mitk::RotationOperation::~RotationOperation(void)
{
}

mitk::ScalarType mitk::RotationOperation::GetAngleOfRotation()
{
  return this->m_AngleOfRotation;
}

const mitk::Point3D mitk::RotationOperation::GetCenterOfRotation()
{
  return this->m_PointOfRotation;
}
const mitk::Vector3D mitk::RotationOperation::GetVectorOfRotation()
{
  return this->m_VectorOfRotation;
}
