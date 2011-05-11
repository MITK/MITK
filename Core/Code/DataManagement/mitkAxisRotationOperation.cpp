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


#include "mitkAxisRotationOperation.h"

namespace mitk
{

AxisRotationOperation
::AxisRotationOperation( OperationType operationType, Point3D center, Vector3D xaxis, Vector3D yaxis, float width, float height, Vector3D spacing )
: PointOperation(operationType, center), m_XAxis( xaxis ), m_YAxis( yaxis), m_Width( width ), m_Height( height ), m_Spacing( spacing )
{
}

AxisRotationOperation
::~AxisRotationOperation()
{
}

Vector3D AxisRotationOperation::GetXAxis()
{
  return m_XAxis;
}

Vector3D AxisRotationOperation::GetYAxis()
{
  return m_YAxis;
}

Vector3D AxisRotationOperation::GetSpacing()
{
  return m_Spacing;
}

float AxisRotationOperation::GetWidth()
{
  return m_Width;
}

float AxisRotationOperation::GetHeight()
{
  return m_Height;
}


} // namespace mitk

