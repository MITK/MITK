/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 21:01:03 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 17190 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkKeyEvent.h"
#include "mitkBaseRenderer.h"

mitk::KeyEvent::KeyEvent(mitk::BaseRenderer* sender, int type, int button, int buttonState, int key, std::string text, const mitk::Point2D& displPosition)
  : Event(sender, type, button, buttonState, key), m_DisplayPosition(displPosition), m_WorldPositionIsSet(false), m_Key(key), m_Text(text)
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
