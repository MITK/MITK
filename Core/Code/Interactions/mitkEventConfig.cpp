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

#include "mitkEventConfig.h"
#include <vtkXMLDataElement.h>
#include <mitkStandardFileLocations.h>
#include <vtkObjectFactory.h>
#include <algorithm>
#include <sstream>
#include "mitkEventFactory.h"
#include "mitkInteractionEvent.h"
#include "mitkInternalEvent.h"
#include "mitkInteractionEventConst.h"
// us
#include "mitkModule.h"
#include "mitkModuleResource.h"
#include "mitkModuleResourceStream.h"
#include "mitkModuleRegistry.h"

namespace mitk
{
  vtkStandardNewMacro(EventConfig);
}

mitk::EventConfig::EventConfig() :
    m_errors(false)
{
  if (m_PropertyList.IsNull())
  {
    m_PropertyList = PropertyList::New();
  }
}

mitk::EventConfig::~EventConfig()
{
}

void mitk::EventConfig::InsertMapping(EventMapping mapping)
{
  for (EventListType::iterator it = m_EventList.begin(); it != m_EventList.end(); ++it)
  {
    if ((*it).interactionEvent->MatchesTemplate(mapping.interactionEvent))
    {
      MITK_INFO<< "Configuration overwritten:" << (*it).variantName;
      m_EventList.erase(it);
      break;
    }
  }
  m_EventList.push_back(mapping);
}

/**
 * @brief Loads the xml file filename and generates the necessary instances.
 **/
bool mitk::EventConfig::LoadConfig(std::string fileName, std::string moduleName)
{
  mitk::Module* module = mitk::ModuleRegistry::GetModule(moduleName);
  mitk::ModuleResource resource = module->GetResource("Interactions/" + fileName);
  if (!resource.IsValid())
  {
    mitkThrow()<< ("Resource not valid. State machine pattern not found:" + fileName);
  }
  mitk::ModuleResourceStream stream(resource);
  this->SetStream(&stream);
  return this->Parse() && !m_errors;
}

void mitk::EventConfig::StartElement(const char* elementName, const char **atts)
{
  std::string name(elementName);

  if (name == CONFIG)
  {
    //
  }
  else if (name == PARAM)
  {
    std::string name = ReadXMLStringAttribut(NAME, atts);
    std::string value = ReadXMLStringAttribut(VALUE, atts);
    m_PropertyList->SetStringProperty(name.c_str(), value.c_str());
  }
  else if (name == INPUT)
  {
    std::string eventClass = ReadXMLStringAttribut(EVENTCLASS, atts);
    std::string eventVariant = ReadXMLStringAttribut(EVENTVARIANT, atts);
    // New list in which all parameters are stored that are given within the <input/> tag
    m_EventPropertyList = PropertyList::New();
    m_EventPropertyList->SetStringProperty(EVENTCLASS.c_str(), eventClass.c_str());
    m_EventPropertyList->SetStringProperty(EVENTVARIANT.c_str(), eventVariant.c_str());
    m_CurrEventMapping.variantName = eventVariant;
  }
  else if (name == ATTRIBUTE)
  {
    // Attributes that describe an Input Event, such as which MouseButton triggered the event,or which modifier keys are pressed
    std::string name = ReadXMLStringAttribut(NAME, atts);
    std::string value = ReadXMLStringAttribut(VALUE, atts);
    m_EventPropertyList->SetStringProperty(name.c_str(), value.c_str());
  }
}

void mitk::EventConfig::EndElement(const char* elementName)
{
  std::string name(elementName);
  // At end of input section, all necessary infos are collected to created an interaction event.
  if (name == INPUT)
  {
    InteractionEvent::Pointer event = EventFactory::CreateEvent(m_EventPropertyList);
    if (event.IsNotNull())
    {
      m_CurrEventMapping.interactionEvent = event;
      InsertMapping(m_CurrEventMapping);
    }
    else
    {
      MITK_WARN<< "EventConfig: Unknown Event-Type in config. Entry skipped: " << name;
    }
  }
}

std::string mitk::EventConfig::ReadXMLStringAttribut(std::string name, const char** atts)
{
  if (atts)
  {
    const char** attsIter = atts;

    while (*attsIter)
    {
      if (name == *attsIter)
      {
        attsIter++;
        return *attsIter;
      }
      attsIter++;
      attsIter++;
    }
  }
  return std::string();
}

const mitk::PropertyList::Pointer mitk::EventConfig::GetPropertyList()
{
  return m_PropertyList;
}

std::string mitk::EventConfig::GetMappedEvent(InteractionEvent* interactionEvent)
{
  // internal events are excluded from mapping
  if (interactionEvent->GetEventClass() == "InternalEvent") {
    InternalEvent* internalEvent = dynamic_cast<InternalEvent*>(interactionEvent);
    return internalEvent->GetSignalName();
  }

  for (EventListType::iterator it = m_EventList.begin(); it != m_EventList.end(); ++it)
  {
    if ((*it).interactionEvent->MatchesTemplate(interactionEvent))
    {
      return (*it).variantName;
    }
  }
  return "";
}

void mitk::EventConfig::ClearConfig()
{
  m_EventList.clear();
}

bool mitk::EventConfig::ReadXMLBooleanAttribut(std::string name, const char** atts)
{
  std::string s = ReadXMLStringAttribut(name, atts);
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);
  if (s == "TRUE")
    return true;
  else
    return false;
}
