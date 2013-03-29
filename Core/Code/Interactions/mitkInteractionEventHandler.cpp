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

#include "mitkInteractionEventHandler.h"
#include "mitkInteractionEvent.h"

mitk::InteractionEventHandler::InteractionEventHandler()
  : m_EventConfig()
{
}

mitk::InteractionEventHandler::~InteractionEventHandler()
{
}

bool mitk::InteractionEventHandler::LoadEventConfig(const std::string& filename, const std::string& moduleName)
{
  // notify sub-classes that new config is set
  bool success = m_EventConfig.LoadConfig(filename, moduleName);
  ConfigurationChanged();
  return success;
}

bool mitk::InteractionEventHandler::AddEventConfig(const std::string& filename, const std::string& moduleName)
{
  if (!m_EventConfig.IsValid())
  {
    MITK_ERROR<< "LoadEventConfig has to be called before AddEventConfig can be used.";
    return false;
  }
  // notify sub-classes that new config is set
  bool success = m_EventConfig.LoadConfig(filename, moduleName);
  ConfigurationChanged();
  return success;
}

mitk::PropertyList::Pointer mitk::InteractionEventHandler::GetAttributes() const
{
  if (m_EventConfig.IsValid())
  {
    return m_EventConfig.GetAttributes();
  }
  else
  {
    MITK_ERROR << "InteractionEventHandler::GetAttributes() requested, but not configuration loaded.";
    return NULL;
  }
}

std::string mitk::InteractionEventHandler::MapToEventVariant(InteractionEvent* interactionEvent)
{
  if (m_EventConfig.IsValid())
  {
    return m_EventConfig.GetMappedEvent(interactionEvent);
  }
  else
  {
    return "";
  }
}

void mitk::InteractionEventHandler::ConfigurationChanged()
{
}
