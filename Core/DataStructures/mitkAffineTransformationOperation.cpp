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


#include "mitkAffineTransformationOperation.h"

mitk::AffineTransformationOperation::AffineTransformationOperation(OperationType operationType,Point3D point, ScalarType angle, int index)
  : mitk::PointOperation(operationType, point, index), m_Angle(angle)
{
}

mitk::AffineTransformationOperation::~AffineTransformationOperation(void)
{
}

mitk::ScalarType mitk::AffineTransformationOperation::GetAngle()
{
  return m_Angle;
}

