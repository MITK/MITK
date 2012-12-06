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

#include "mitkException.h"
#include "mitkInteractionEvent.h"

mitk::InteractionEvent::InteractionEvent(BaseRenderer* baseRenderer, std::string eventClass) :
    m_Sender(baseRenderer), m_EventClass(eventClass)
{
}

void mitk::InteractionEvent::SetSender(mitk::BaseRenderer* sender)
{
  m_Sender = sender;
}

const mitk::BaseRenderer* mitk::InteractionEvent::GetSender()
{
  return m_Sender;
}

bool mitk::InteractionEvent::isEqual(InteractionEvent::Pointer)
{
  return false;
}

mitk::InteractionEvent::~InteractionEvent()
{
}

std::string mitk::InteractionEvent::GetEventClass()
{
  return m_EventClass;
}
