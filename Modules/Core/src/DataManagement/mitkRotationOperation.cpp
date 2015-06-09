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


#include "mitkRotationOperation.h"

mitk::RotationOperation::RotationOperation(OperationType operationType, Point3D pointOfRotation, Vector3D vectorOfRotation, ScalarType angleOfRotation)
  : mitk::Operation(operationType)
  , m_AngleOfRotation(angleOfRotation), m_PointOfRotation(pointOfRotation), m_VectorOfRotation(vectorOfRotation)
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
