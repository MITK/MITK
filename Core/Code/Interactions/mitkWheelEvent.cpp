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


#include "mitkWheelEvent.h"
#include "mitkNumericTypes.h"

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
