/*=========================================================================
 
 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "berryDebugBreakpointManager.h"
#include "berryDebugUtil.h"
#include "berryObject.h"

#include "berryLog.h"

#include <Poco/NumberParser.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/DOMWriter.h>
#include <Poco/SAX/InputSource.h>
#include <Poco/SAX/SAXException.h>

#include <Poco/FileStream.h>

#include <sstream>

namespace berry
{

const std::string DebugBreakpointManager::BREAKPOINTS_XML = "breakpoints.xml";

const std::string DebugBreakpointManager::BREAKPOINTS_TAG = "breakpoints";
const std::string DebugBreakpointManager::OBJECT_TAG = "object";
const std::string DebugBreakpointManager::SMARTPOINTER_TAG = "smartpointer";

const std::string DebugBreakpointManager::ID_ATTR = "id";
const std::string DebugBreakpointManager::CLASSNAME_ATTR = "className";
const std::string DebugBreakpointManager::OBJECTID_ATTR = "objectId";
const std::string DebugBreakpointManager::ENABLED_ATTR = "enabled";

void DebugBreakpointManager::AddSmartpointerBreakpoint(int smartPointerId,
    const Object* obj)
{
  BERRY_INFO << "Smartpointer breakpoint added: " << smartPointerId;
  m_SmartPointerBreakpoints[smartPointerId] = obj;
}

void DebugBreakpointManager::AddObjectBreakpoint(unsigned long objectTraceId)
{
  BERRY_INFO << "Object breakpoint added: " << objectTraceId;
  m_ObjectBreakpoints.insert(objectTraceId);
}

void DebugBreakpointManager::RemoveSmartpointerBreakpoint(int smartPointerId)
{
  m_SmartPointerBreakpoints.erase(smartPointerId);
}

void DebugBreakpointManager::RemoveObjectBreakpoint(unsigned long objectTraceId)
{
  m_ObjectBreakpoints.erase(objectTraceId);
}

const std::set<unsigned long>& DebugBreakpointManager::GetObjectBreakpoints() const
{
  return m_ObjectBreakpoints;
}

const Poco::HashMap<int, const Object*>& DebugBreakpointManager::GetSmartPointerBreakpoints() const
{
  return m_SmartPointerBreakpoints;
}

bool DebugBreakpointManager::BreakAtObject(unsigned long traceId) const
{
  return m_ObjectBreakpoints.find(traceId) != m_ObjectBreakpoints.end();
}

bool DebugBreakpointManager::BreakAtSmartpointer(int spId) const
{
  return m_SmartPointerBreakpoints.find(spId)
      != m_SmartPointerBreakpoints.end();
}

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
void DebugBreakpointManager::SaveState(const Poco::Path& path) const
{
  Poco::XML::Document* doc = new Poco::XML::Document();
  Poco::XML::Element* breakpoints = doc->createElement(BREAKPOINTS_TAG);
  doc->appendChild(breakpoints)->release();

  for (std::set<unsigned long>::const_iterator i = m_ObjectBreakpoints.begin(); i
      != m_ObjectBreakpoints.end(); ++i)
  {
    Poco::XML::Element* objectBreakpoint = doc->createElement(OBJECT_TAG);
    breakpoints->appendChild(objectBreakpoint)->release();
    std::stringstream ss;
    ss << *i;
    objectBreakpoint->setAttribute(ID_ATTR, ss.str());

    const Object* obj = DebugUtil::GetObject(*i);
    if (obj)
    {
      objectBreakpoint->setAttribute(CLASSNAME_ATTR, obj->GetClassName());

    }
  }

  for (Poco::HashMap<int, const Object*>::ConstIterator i =
      m_SmartPointerBreakpoints.begin(); i != m_SmartPointerBreakpoints.end(); ++i)
  {
    Poco::XML::Element* spBreakpoint = doc->createElement(SMARTPOINTER_TAG);
    breakpoints->appendChild(spBreakpoint)->release();
    std::stringstream ss;
    ss << i->first;
    spBreakpoint->setAttribute(ID_ATTR, ss.str());

    const Object* obj = i->second;
    if (i->second)
    {
      spBreakpoint->setAttribute(CLASSNAME_ATTR, obj->GetClassName());
      ss.clear();
      ss << obj->GetTraceId();
      spBreakpoint->setAttribute(OBJECTID_ATTR, ss.str());
    }
  }

  Poco::FileOutputStream writer(path.toString());
  Poco::XML::DOMWriter out;
  out.setOptions(3); //write declaration and pretty print
  out.writeNode(writer, doc);

  doc->release();
}
#else
void DebugBreakpointManager::SaveState(const Poco::Path& /*path*/) const
{}
#endif

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
void DebugBreakpointManager::RestoreState(const Poco::Path& path)
{
  try
  {
    Poco::XML::DOMParser parser;

    Poco::FileInputStream reader(path.toString());
    Poco::XML::InputSource source(reader);

    //source.setSystemId(baseDir);
    Poco::XML::Document* doc = parser.parse(&source);
    Poco::XML::Element* breakpoints = doc->documentElement();

    if (breakpoints)
    {
      // restore object breakpoints
      Poco::XML::NodeList* elementList = breakpoints->getElementsByTagName(
          OBJECT_TAG);
      for (std::size_t i = 0; i < elementList->length(); i++)
      {
        Poco::XML::Element* elem =
            dynamic_cast<Poco::XML::Element*> (elementList->item(i));

        if (!elem->hasAttribute(ID_ATTR))
          continue;

        const std::string& attr = elem->getAttribute(ID_ATTR);

        int traceId = 0;
        try
        {
          traceId = Poco::NumberParser::parse(attr);
        } catch (const Poco::SyntaxException& e)
        {
          BERRY_WARN << e.displayText();
        }

        DebugUtil::GetBreakpointManager()->AddObjectBreakpoint(traceId);
      }
      elementList->release();

      // restore smartpointer breakpoints
      elementList = breakpoints->getElementsByTagName(SMARTPOINTER_TAG);
      for (std::size_t i = 0; i < elementList->length(); i++)
      {
        Poco::XML::Element* elem =
            dynamic_cast<Poco::XML::Element*> (elementList->item(i));

        if (!elem->hasAttribute(ID_ATTR))
          continue;

        const std::string& attr = elem->getAttribute(ID_ATTR);

        int spId = 0;
        try
        {
          spId = Poco::NumberParser::parse(attr);
        } catch (const Poco::SyntaxException& e)
        {
          BERRY_WARN << e.displayText();
        }

        DebugUtil::GetBreakpointManager()->AddSmartpointerBreakpoint(spId);
      }
      elementList->release();
    }

    doc->release();
  } catch (Poco::XML::SAXParseException& e)
  {
    BERRY_WARN << e.displayText();
  }
  catch (Poco::FileNotFoundException& /*e*/)
  {

  }
  catch (Poco::FileException& e)
  {
    BERRY_WARN << e.displayText();
  }
}
#else
void DebugBreakpointManager::RestoreState(const Poco::Path& /*path*/)
{
}
#endif

}
