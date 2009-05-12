/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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
