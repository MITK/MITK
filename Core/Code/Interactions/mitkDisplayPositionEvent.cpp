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


#include "mitkDisplayPositionEvent.h"
#include "mitkBaseRenderer.h"

mitk::DisplayPositionEvent::DisplayPositionEvent(mitk::BaseRenderer* sender, int type, int button, int buttonState, int key, const mitk::Point2D& displPosition)
  : Event(sender, type, button, buttonState, key), m_DisplayPosition(displPosition), m_WorldPositionIsSet(false)
{
}

const mitk::Point3D& mitk::DisplayPositionEvent::GetWorldPosition() const
{
  if(m_WorldPositionIsSet)
    return m_WorldPosition;
  m_WorldPositionIsSet = true;
  assert(m_Sender!=NULL);
  m_Sender->PickWorldPoint(m_DisplayPosition, m_WorldPosition);
  return m_WorldPosition;
}
