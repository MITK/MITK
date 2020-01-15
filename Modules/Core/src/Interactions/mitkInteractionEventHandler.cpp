/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkInteractionEventHandler.h"
#include "mitkInteractionEvent.h"

mitk::InteractionEventHandler::InteractionEventHandler() : m_EventConfig()
{
}

mitk::InteractionEventHandler::~InteractionEventHandler()
{
}

bool mitk::InteractionEventHandler::SetEventConfig(const std::string &filename, const us::Module *module)
{
  EventConfig newConfig(filename, module);
  if (newConfig.IsValid())
  {
    m_EventConfig = newConfig;
    // notify sub-classes that new config is set
    ConfigurationChanged();
    return true;
  }
  MITK_ERROR << "InteractionEventHandler: Failed to load EventConfiguration";
  return false;
}

bool mitk::InteractionEventHandler::SetEventConfig(const EventConfig &config)
{
  if (config.IsValid())
  {
    m_EventConfig = config;
    // notify sub-classes that new config is set
    ConfigurationChanged();
    return true;
  }
  return false;
}

mitk::EventConfig mitk::InteractionEventHandler::GetEventConfig() const
{
  return m_EventConfig;
}

bool mitk::InteractionEventHandler::AddEventConfig(const std::string &filename, const us::Module *module)
{
  if (!m_EventConfig.IsValid())
  {
    MITK_ERROR << "SetEventConfig has to be called before AddEventConfig can be used.";
    return false;
  }
  // notify sub-classes that new config is set
  bool success = m_EventConfig.AddConfig(filename, module);
  if (success)
  {
    ConfigurationChanged();
  }
  return success;
}

bool mitk::InteractionEventHandler::AddEventConfig(const EventConfig &config)
{
  if (!m_EventConfig.IsValid())
  {
    MITK_ERROR << "SetEventConfig has to be called before AddEventConfig can be used.";
    return false;
  }
  // notify sub-classes that new config is set
  bool success = m_EventConfig.AddConfig(config);
  if (success)
  {
    ConfigurationChanged();
  }
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
    return nullptr;
  }
}

std::string mitk::InteractionEventHandler::MapToEventVariant(InteractionEvent *interactionEvent)
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
