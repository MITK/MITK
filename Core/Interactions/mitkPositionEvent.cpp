/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#include "mitkPositionEvent.h"
#include "mitkVector.h"

//##ModelId=3E5B7CF900F0
mitk::PositionEvent::PositionEvent(mitk::BaseRenderer* sender, int type, int button, int buttonState, int key, const mitk::Point2D& displPosition, const mitk::Point3D& worldPosition)
: DisplayPositionEvent(sender, type, button, buttonState, key, displPosition), m_WorldPosition(worldPosition)
{}

//##ModelId=3EDC8EEC00F8
const mitk::Point3D& mitk::PositionEvent::GetWorldPosition() const
{
	return m_WorldPosition;
}

//##ModelId=3EDC8EEC0136
void mitk::PositionEvent::SetWorldPosition(const mitk::Point3D& worldPosition)
{
	m_WorldPosition = worldPosition;
}
