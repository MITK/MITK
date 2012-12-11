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

#include "mitkEventHandler.h"
#include "mitkInteractionEvent.h"

mitk::EventHandler::EventHandler()
{
  m_EventConfig = NULL;
}

mitk::EventHandler::~EventHandler()
{
  if (m_EventConfig != NULL)
  {
    m_EventConfig->Delete();
  }
}

bool mitk::EventHandler::LoadEventConfig()
{
  // TODO:: implement standard file location, for standard events
  return false;
}

bool mitk::EventHandler::LoadEventConfig(std::string filename)
{
  if (m_EventConfig != NULL)
  {
    m_EventConfig->Delete();
  }
  m_EventConfig = EventConfig::New();
  return m_EventConfig->LoadConfig(filename);
}

std::string mitk::EventHandler::GetMappedEvent(InteractionEvent* interactionEvent)
{
  if (m_EventConfig != NULL)
  {
    return m_EventConfig->GetMappedEvent(interactionEvent);
  }
  else
  {
    return "";
  }
}
