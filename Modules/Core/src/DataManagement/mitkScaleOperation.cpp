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
