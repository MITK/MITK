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

#include "mitkInternalEvent.h"

mitk::InternalEvent::InternalEvent(mitk::BaseRenderer* baseRenderer, std::string signalName) :
    InteractionEvent(baseRenderer, "InternalEvent"), m_SignalName(signalName)
{
}

bool mitk::InternalEvent::isEqual(mitk::InteractionEvent::Pointer interactionEvent)
{
  mitk::InternalEvent* internalEvent = dynamic_cast<mitk::InternalEvent*>(interactionEvent.GetPointer());
  if (internalEvent == NULL)
  {
    return false;
  }
  return (m_SignalName == internalEvent->GetSignalName());
}

mitk::InternalEvent::~InternalEvent()
{
}

std::string mitk::InternalEvent::GetSignalName()
{
  return m_SignalName;
}
