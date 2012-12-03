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

mitk::InteractionKeyEvent::InteractionKeyEvent(mitk::BaseRenderer* baseRenderer, char key,
    mitk::EModifiers modifiers = MOD_NoModifiers) :
    InteractionEvent(baseRenderer, "MouseWheelEvent"), m_Key(key),m_Modifiers(modifiers)
{
}

mitk::EModifiers mitk::InteractionKeyEvent::GetModifiers()
{
  return m_Modifiers;
}

char mitk::InteractionKeyEvent::GetKey()
{
  return m_Key;
}

mitk::InteractionKeyEvent::~InteractionKeyEvent()
{
}

bool mitk::InteractionKeyEvent::isEqual(mitk::InteractionEvent::Pointer interactionEvent)
{
  mitk::InteractionKeyEvent* mpe = dynamic_cast< mitk::InteractionKeyEvent* >(interactionEvent.GetPointer());
  if (mpe == NULL)
  {
    return false;
  }
  if (this->GetModifiers() != mpe->GetModifiers())
  {
    return false;
  }
  if (this->GetKey() != mpe->GetKey())
  {
    return false;
  }
  return true;
}

