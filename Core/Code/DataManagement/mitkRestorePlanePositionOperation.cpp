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


#include "mitkRestorePlanePositionOperation.h"

namespace mitk
{

RestorePlanePositionOperation
::RestorePlanePositionOperation( OperationType operationType, float width, float height, Vector3D spacing , unsigned int pos, Vector3D direction, AffineTransform3D::Pointer transform/*, PlaneOrientation orientation*/)
: Operation(operationType), m_Spacing( spacing ), m_DirectionVector(direction), m_Width( width ), m_Height( height ), m_Pos(pos), m_Transform(transform)/*, m_Orientation(orientation)*/
{
  //MITK_INFO<<"Width: "<<width<<"height: "<<height<<"spacing: "<<spacing<<"direction: "<<direction<<"transform: "<<transform;
}

RestorePlanePositionOperation
::~RestorePlanePositionOperation()
{
}

Vector3D RestorePlanePositionOperation::GetSpacing()
{
  return m_Spacing;
}

Vector3D RestorePlanePositionOperation::GetDirectionVector()
{
  return m_DirectionVector;
}

float RestorePlanePositionOperation::GetWidth()
{
  return m_Width;
}

float RestorePlanePositionOperation::GetHeight()
{
  return m_Height;
}

unsigned int RestorePlanePositionOperation::GetPos()
{
  return m_Pos;
}

AffineTransform3D::Pointer RestorePlanePositionOperation::GetTransform()
{
  return m_Transform;
}

} // namespace mitk

