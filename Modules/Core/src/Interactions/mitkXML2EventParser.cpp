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

#include "mitkXML2EventParser.h"

#include "mitkEventFactory.h"
#include "mitkInteractionEvent.h"
#include "mitkInteractionEventConst.h"
#include "mitkInteractionKeyEvent.h"
#include "mitkInternalEvent.h"

// VTK
#include <vtkXMLDataElement.h>

// us
#include "usGetModuleContext.h"
#include "usModule.h"
#include "usModuleResource.h"
#include "usModuleResourceStream.h"

namespace mitk
{
  void mitk::XML2EventParser::StartElement(const char *elementName, const char **atts)
  {
    std::string name(elementName);

    if (name == InteractionEventConst::xmlTagConfigRoot())
    {
      //
    }
    else if (name == InteractionEventConst::xmlTagEventVariant())
    {
      std::string eventClass = ReadXMLStringAttribute(InteractionEventConst::xmlParameterEventClass(), atts);
      // New list in which all parameters are stored that are given within the <input/> tag
      m_EventPropertyList = PropertyList::New();
      m_EventPropertyList->SetStringProperty(InteractionEventConst::xmlParameterEventClass().c_str(),
                                             eventClass.c_str());
      // MITK_INFO << "event class " << eventClass;
    }
    else if (name == InteractionEventConst::xmlTagAttribute())
    {
      // Attributes that describe an Input Event, such as which MouseButton triggered the event,or which modifier keys
      // are
      // pressed
      std::string name = ReadXMLStringAttribute(InteractionEventConst::xmlParameterName(), atts);
      std::string value = ReadXMLStringAttribute(InteractionEventConst::xmlParameterValue(), atts);
      // MITK_INFO << "tag attr " << value;
      m_EventPropertyList->SetStringProperty(name.c_str(), value.c_str());
    }
  }

  void mitk::XML2EventParser::EndElement(const char *elementName)
  {
    std::string name(elementName);
    // At end of input section, all necessary infos are collected to created an interaction event.
    if (name == InteractionEventConst::xmlTagEventVariant())
    {
      InteractionEvent::Pointer event = EventFactory::CreateEvent(m_EventPropertyList);
      if (event.IsNotNull())
      {
        m_InteractionList.push_back(event);
      }
      else
      {
        MITK_WARN << "EventConfig: Unknown Event-Type in config. Entry skipped: " << name;
      }
    }
  }

  std::string mitk::XML2EventParser::ReadXMLStringAttribute(const std::string &name, const char **atts)
  {
    if (atts)
    {
      const char **attsIter = atts;

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

  bool mitk::XML2EventParser::ReadXMLBooleanAttribute(const std::string &name, const char **atts)
  {
    std::string s = ReadXMLStringAttribute(name, atts);
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);

    return s == "TRUE";
  }

  mitk::XML2EventParser::XML2EventParser(const std::string &filename, const us::Module *module)
  {
    if (module == nullptr)
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

    us::ModuleResourceStream stream(resource);
    this->SetStream(&stream);
    bool success = this->Parse();
    if (!success)
      MITK_ERROR << "Error occured during parsing of EventXML File.";
  }

  mitk::XML2EventParser::XML2EventParser(std::istream &inputStream)
  {
    this->SetStream(&inputStream);
    bool success = this->Parse();
    if (!success)
      MITK_ERROR << "Error occured during parsing of EventXML File.";
  }
}
