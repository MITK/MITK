/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkWheelEvent.h"
#include "mitkVector.h"

mitk::WheelEvent::WheelEvent(mitk::BaseRenderer* sender, int type, int button, int buttonState, int key, const mitk::Point2D& displPosition, int delta)
: DisplayPositionEvent(sender, type, button, buttonState, key, displPosition)
{
  //m_WorldPosition = worldPosition;
  //m_WorldPositionIsSet = true;
  m_Delta = delta;
}
int mitk::WheelEvent::GetDelta() const
{
  return m_Delta;
}
