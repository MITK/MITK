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


#include "mitkKeyEvent.h"
#include "mitkBaseRenderer.h"

mitk::KeyEvent::KeyEvent(mitk::BaseRenderer* sender, int type, int button, int buttonState, int key, std::string text, const mitk::Point2D& displPosition)
: Event(sender, type, button, buttonState, key)
, m_DisplayPosition(displPosition)
, m_Key(key)
, m_Text(text)
, m_WorldPositionIsSet(false)
{
}

const mitk::Point3D& mitk::KeyEvent::GetWorldPosition() const
{
  if(m_WorldPositionIsSet)
    return m_WorldPosition;
  m_WorldPositionIsSet = true;
  assert(m_Sender!=NULL);
  m_Sender->PickWorldPoint(m_DisplayPosition, m_WorldPosition);
  return m_WorldPosition;
}
