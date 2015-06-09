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


#include "mitkPositionEvent.h"
#include "mitkNumericTypes.h"

mitk::PositionEvent::PositionEvent(mitk::BaseRenderer* sender, int type, int button, int buttonState, int key, const mitk::Point2D& displPosition, const mitk::Point3D& worldPosition)
: DisplayPositionEvent(sender, type, button, buttonState, key, displPosition)
{
  m_WorldPosition = worldPosition;
  m_WorldPositionIsSet = true;
}
