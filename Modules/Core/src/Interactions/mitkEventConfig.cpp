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

#include "mitkEventFactory.h"
#include "mitkInteractionEvent.h"
#include "mitkInternalEvent.h"
#include "mitkInteractionKeyEvent.h"
#include "mitkInteractionEventConst.h"

// VTK
#include <vtkXMLParser.h>
#include <vtkXMLDataElement.h>

// us
#include "usGetModuleContext.h"
#include "usModule.h"
#include "usModuleResource.h"
#include "usModuleResourceStream.h"

namespace mitk {

class EventConfigXMLParser : public vtkXMLParser
{

public:

  EventConfigXMLParser(EventConfigPrivate* d);

protected:

  /**
   * @brief Derived from XMLReader
   **/
  void StartElement(const char* elementName, const char **atts) override;

  /**
   * @brief Derived from XMLReader
   **/
  void EndElement(const char* elementName) override;

  std::string ReadXMLStringAttribute(const std::string& name, const char** atts);
  bool ReadXMLBooleanAttribute(const std::string& name, const char** atts);

private:

  EventConfigPrivate* const d;

};

struct EventConfigPrivate : public us::SharedData
{

  EventConfigPrivate();
  EventConfigPrivate(const EventConfigPrivate& other);

  struct EventMapping
  {
    std::string variantName;
    InteractionEvent::ConstPointer interactionEvent;
  };

  typedef std::list<EventMapping> EventListType;

  /**
   * Checks if mapping with the same parameters already exists, if so, it is replaced,
   * else the new mapping added
   */
  void InsertMapping(const EventMapping& mapping);


  void CopyMapping( const EventListType );

  /**
   * @brief List of all global properties of the config object.
   */
  PropertyList::Pointer m_PropertyList;

  /**
   * @brief Temporal list of all prMousePressEventoperties of a Event. Used to parse an Input-Event and collect all parameters between the two <input>
   * and </event_variant> tags.
   */
  PropertyList::Pointer m_EventPropertyList;

  EventMapping m_CurrEventMapping;


  /**
   * Stores InteractionEvents and their corresponding VariantName
   */
  EventListType m_EventList;

  bool m_Errors; // use member, because of inheritance from vtkXMLParser we can't return a success value for parsing the file.

  EventConfigXMLParser m_XmlParser;
};

}


mitk::EventConfigPrivate::EventConfigPrivate()
  : m_PropertyList(PropertyList::New())
  , m_EventPropertyList( PropertyList::New() )
  , m_Errors(false)
  , m_XmlParser(this)
{
  // Avoid VTK warning: Trying to delete object with non-zero reference count.
  m_XmlParser.SetReferenceCount(0);
}

mitk::EventConfigPrivate::EventConfigPrivate(const EventConfigPrivate& other)
  : us::SharedData(other)
  , m_PropertyList(other.m_PropertyList->Clone())
  , m_EventPropertyList(other.m_EventPropertyList->Clone())
  , m_CurrEventMapping(other.m_CurrEventMapping)
  , m_EventList(other.m_EventList)
  , m_Errors(other.m_Errors)
  , m_XmlParser(this)
{
  // Avoid VTK warning: Trying to delete object with non-zero reference count.
  m_XmlParser.SetReferenceCount(0);
}

void mitk::EventConfigPrivate::InsertMapping(const EventMapping& mapping)
{
  for (EventListType::iterator it = m_EventList.begin(); it != m_EventList.end(); ++it)
  {
    if (*(it->interactionEvent) == *mapping.interactionEvent)
    {
      //MITK_INFO<< "Configuration overwritten:" << (*it).variantName;
      m_EventList.erase(it);
      break;
    }
  }
  m_EventList.push_back(mapping);
}

void mitk::EventConfigPrivate::CopyMapping( const EventListType eventList )
{
  EventListType::const_iterator iter;
  for( iter=eventList.begin(); iter!=eventList.end(); ++iter )
  {
    InsertMapping( *(iter) );
  }
}


mitk::EventConfigXMLParser::EventConfigXMLParser(EventConfigPrivate *d)
  : d(d)
{
}

void mitk::EventConfigXMLParser::StartElement(const char* elementName, const char **atts)
{
  std::string name(elementName);

  if (name == InteractionEventConst::xmlTagConfigRoot())
  {
    //
  }
  else if (name == InteractionEventConst::xmlTagParam())
  {
    const std::string name = ReadXMLStringAttribute(InteractionEventConst::xmlParameterName(), atts);
    const std::string value = ReadXMLStringAttribute(InteractionEventConst::xmlParameterValue(), atts);
    d->m_PropertyList->SetStringProperty(name.c_str(), value.c_str());
  }
  else if (name == InteractionEventConst::xmlTagEventVariant())
  {
    const std::string eventClass = ReadXMLStringAttribute(InteractionEventConst::xmlParameterEventClass(), atts);
    const std::string eventVariant = ReadXMLStringAttribute(InteractionEventConst::xmlParameterName(), atts);
    // New list in which all parameters are stored that are given within the <input/> tag
    d->m_EventPropertyList = PropertyList::New();
    d->m_EventPropertyList->SetStringProperty(InteractionEventConst::xmlParameterEventClass().c_str(), eventClass.c_str());
    d->m_EventPropertyList->SetStringProperty(InteractionEventConst::xmlParameterEventVariant().c_str(), eventVariant.c_str());
    d->m_CurrEventMapping.variantName = eventVariant;
  }
  else if (name == InteractionEventConst::xmlTagAttribute())
  {
    // Attributes that describe an Input Event, such as which MouseButton triggered the event,or which modifier keys are pressed
    const std::string name = ReadXMLStringAttribute(InteractionEventConst::xmlParameterName(), atts);
    const std::string value = ReadXMLStringAttribute(InteractionEventConst::xmlParameterValue(), atts);
    d->m_EventPropertyList->SetStringProperty(name.c_str(), value.c_str());
  }
}

void mitk::EventConfigXMLParser::EndElement(const char* elementName)
{
  const std::string name(elementName);
  // At end of input section, all necessary infos are collected to created an interaction event.
  if (name == InteractionEventConst::xmlTagEventVariant())
  {
    InteractionEvent::Pointer event = EventFactory::CreateEvent(d->m_EventPropertyList);
    if (event.IsNotNull())
    {
      d->m_CurrEventMapping.interactionEvent = event;
      d->InsertMapping(d->m_CurrEventMapping);
    }
    else
    {
      MITK_WARN<< "EventConfig: Unknown Event-Type in config. Entry skipped: " << name;
    }
  }
}

std::string mitk::EventConfigXMLParser::ReadXMLStringAttribute(const std::string& name, const char** atts)
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
      attsIter += 2;
    }
  }
  return std::string();
}

bool mitk::EventConfigXMLParser::ReadXMLBooleanAttribute(const std::string& name, const char** atts)
{
  std::string s = ReadXMLStringAttribute(name, atts);
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);

  return s == "TRUE";
}

mitk::EventConfig::EventConfig()
  : d(new EventConfigPrivate)
{
}

mitk::EventConfig::EventConfig(const EventConfig &other)
  : d(other.d)
{
}

mitk::EventConfig::EventConfig(const std::string& filename, const us::Module* module)
  : d(new EventConfigPrivate)
{
  if (module == NULL)
  {
    module = us::GetModuleContext()->GetModule();
  }
  us::ModuleResource resource = module->GetResource("Interactions/" + filename);
  if (!resource.IsValid())
  {
    MITK_ERROR << "Resource not valid. State machine pattern in module " << module->GetName()
               << " not found: /Interactions/" << filename;
    return;
  }

  EventConfig newConfig;
  us::ModuleResourceStream stream(resource);
  newConfig.d->m_XmlParser.SetStream(&stream);
  bool success = newConfig.d->m_XmlParser.Parse() && !newConfig.d->m_Errors;
  if (success)
  {
    *this = newConfig;
  }
}

mitk::EventConfig::EventConfig(std::istream &inputStream)
 : d(new EventConfigPrivate)
{
  EventConfig newConfig;
  newConfig.d->m_XmlParser.SetStream(&inputStream);
  bool success = newConfig.d->m_XmlParser.Parse() && !newConfig.d->m_Errors;
  if (success)
  {
    *this = newConfig;
  }
}

mitk::EventConfig::EventConfig(const std::vector<PropertyList::Pointer> &configDescription)
: d(new EventConfigPrivate)
{
  std::vector<PropertyList::Pointer>::const_iterator it_end = configDescription.end();
  for (std::vector<PropertyList::Pointer>::const_iterator it = configDescription.begin(); it != it_end; ++it)
  {
    std::string typeVariant;
    (*it)->GetStringProperty(InteractionEventConst::xmlTagEventVariant().c_str(), typeVariant);
    if ( typeVariant != "" )
    {
      InteractionEvent::Pointer event = EventFactory::CreateEvent(*it);
      if (event.IsNotNull())
      {

        d->m_CurrEventMapping.interactionEvent = event;
        std::string eventVariant;
        (*it)->GetStringProperty(InteractionEventConst::xmlTagEventVariant().c_str(), eventVariant);
        d->m_CurrEventMapping.variantName = eventVariant;
        d->InsertMapping(d->m_CurrEventMapping);
      }
      else
      {
        MITK_WARN<< "EventConfig: Unknown Event-Type in config. When constructing from PropertyList.";
      }
    }
    else
    {
      (*it)->GetStringProperty(InteractionEventConst::xmlTagParam().c_str(), typeVariant);
      if ( typeVariant != "" )
      {
        std::string name, value;
        (*it)->GetStringProperty(InteractionEventConst::xmlParameterName().c_str(), name);
        (*it)->GetStringProperty(InteractionEventConst::xmlParameterValue().c_str(), value);
        d->m_PropertyList->SetStringProperty(name.c_str(), value.c_str());
      }
    }
  }
}

mitk::EventConfig& mitk::EventConfig::operator =(const mitk::EventConfig& other)
{
  d = other.d;
  return *this;
}

mitk::EventConfig::~EventConfig()
{
}

bool mitk::EventConfig::IsValid() const
{
  return !( d->m_EventList.empty() && d->m_PropertyList->IsEmpty() );
}

bool mitk::EventConfig::AddConfig(const std::string& fileName, const us::Module* module)
{
  if (module == NULL)
  {
    module = us::GetModuleContext()->GetModule();
  }
  us::ModuleResource resource = module->GetResource("Interactions/" + fileName);
  if (!resource.IsValid())
  {
    MITK_ERROR << "Resource not valid. State machine pattern in module " << module->GetName()
               << " not found: /Interactions/" << fileName;
    return false;
  }

  EventConfig newConfig(*this);
  us::ModuleResourceStream stream(resource);
  newConfig.d->m_XmlParser.SetStream(&stream);
  bool success = newConfig.d->m_XmlParser.Parse() && !newConfig.d->m_Errors;
  if (success)
  {
    *this = newConfig;
  }
  return success;
}

bool mitk::EventConfig::AddConfig(const EventConfig& config)
{
  if (!config.IsValid()) return false;

  d->m_PropertyList->ConcatenatePropertyList(config.d->m_PropertyList->Clone(), true);
  d->m_EventPropertyList = config.d->m_EventPropertyList->Clone();
  d->m_CurrEventMapping = config.d->m_CurrEventMapping;
  d->CopyMapping( config.d->m_EventList );

  return true;
}

mitk::PropertyList::Pointer mitk::EventConfig::GetAttributes() const
{
  return d->m_PropertyList;
}

std::string mitk::EventConfig::GetMappedEvent(const EventType& interactionEvent) const
{
  // internal events are excluded from mapping
  if (std::strcmp(interactionEvent->GetNameOfClass(), "InternalEvent") == 0)
  {
    InternalEvent* internalEvent = dynamic_cast<InternalEvent*>(interactionEvent.GetPointer());
    return internalEvent->GetSignalName();
  }

  for (EventConfigPrivate::EventListType::const_iterator it = d->m_EventList.begin();
       it != d->m_EventList.end(); ++it)
  {
    if (*(it->interactionEvent) == *interactionEvent)
    {
      return (*it).variantName;
    }
  }
  // if this part is reached, no mapping has been found,
  // so here we handle key events and map a key event to the string "Std" + letter/code
  // so "A" will be returned as "StdA"
  if (std::strcmp(interactionEvent->GetNameOfClass(), "InteractionKeyEvent") == 0)
  {
    InteractionKeyEvent* keyEvent = dynamic_cast<InteractionKeyEvent*>(interactionEvent.GetPointer());
    return ("Std" + keyEvent->GetKey());
  }
  return "";
}

void mitk::EventConfig::ClearConfig()
{
  d->m_PropertyList->Clear();
  d->m_EventPropertyList->Clear();
  d->m_CurrEventMapping.variantName.clear();
  d->m_CurrEventMapping.interactionEvent = NULL;
  d->m_EventList.clear();
  d->m_Errors = false;
}
