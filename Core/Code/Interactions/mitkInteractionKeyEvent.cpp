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

#include "mitkInteractionKeyEvent.h"

mitk::InteractionKeyEvent::InteractionKeyEvent(mitk::BaseRenderer* baseRenderer, const std::string key, mitk::ModifierKeys modifiers = ControlKey) :
    InteractionEvent(baseRenderer, "KeyEvent"), m_Key(key), m_Modifiers(modifiers)
{
}

mitk::ModifierKeys mitk::InteractionKeyEvent::GetModifiers() const
{
  return m_Modifiers;
}

std::string mitk::InteractionKeyEvent::GetKey() const
{
  return m_Key;
}


mitk::InteractionKeyEvent::~InteractionKeyEvent()
{
}

bool mitk::InteractionKeyEvent::MatchesTemplate(mitk::InteractionEvent::Pointer interactionEvent)
{
  mitk::InteractionKeyEvent* keyEvent = dynamic_cast<mitk::InteractionKeyEvent*>(interactionEvent.GetPointer());
  if (keyEvent == NULL)
  {
    return false;
  }
  return (this->GetModifiers() == keyEvent->GetModifiers() && this->GetKey() == keyEvent->GetKey());
}
