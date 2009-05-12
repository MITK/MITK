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


#include <mitkDrawOperation.h>


mitk::DrawOperation::DrawOperation(OperationType operationType, Point3D point, Point3D last_point, int draw_state, int radius)
: Operation(operationType), m_Point(point), m_LastPoint(last_point), m_DrawState(draw_state), m_Radius(radius)
{}


mitk::DrawOperation::~DrawOperation()
{
}


mitk::Point3D mitk::DrawOperation::GetPoint()
{
  return m_Point;
}

mitk::Point3D mitk::DrawOperation::GetLastPoint()
{
  return m_LastPoint;
}

int mitk::DrawOperation::GetDrawState()
{
  return m_DrawState;
}

int mitk::DrawOperation::GetRadius()
{
  return m_Radius;
}

mitk::SeedsImage::Pointer mitk::DrawOperation::GetSeedsImage()
{
  return m_SeedsImage;
}

mitk::SeedsImage::Pointer mitk::DrawOperation::GetLastSeedsImage()
{
  return m_LastSeedsImage;
}
