/*=========================================================================

 Program:   openCherry Platform
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

#include "cherryIDebugObjectListener.h"

#include "cherryDebugUtil.h"
#include "cherryObject.h"
#include "cherryLog.h"
#include "cherryPlatform.h"
#include "cherryDebugBreakpointManager.h"

#include <Poco/Bugcheck.h>
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

#include <numeric>

namespace cherry
{

static IDebugObjectListener::Events _G_ObjectEvents;

const std::string DebugUtil::DEBUG_UTIL_XML = "debugutil.xml";
const std::string DebugUtil::DEBUGUTIL_TAG = "debugutil";
const std::string DebugUtil::TRACEOBJECT_TAG = "traceObject";
const std::string DebugUtil::TRACECLASS_TAG = "traceClass";
const std::string DebugUtil::ID_ATTR = "id";
const std::string DebugUtil::NAME_ATTR = "name";

Poco::HashMap<Poco::UInt32, std::list<unsigned int> >
    DebugUtil::m_TraceIdToSmartPointerMap;
Poco::HashMap<Poco::UInt32, const Object*> DebugUtil::m_TraceIdToObjectMap;
std::set<unsigned int> DebugUtil::m_TracedObjects;
std::set<std::string> DebugUtil::m_TracedClasses;

class NotClassName: public std::unary_function<const Object*, bool>
{

  std::string name;

public:
  NotClassName(const std::string& s) :
    name(s)
  {
  }
  bool operator()(Poco::HashMapEntry<unsigned int, const Object*>& entry) const
  {
    return name != entry.second->GetClassName();
  }
};

class AccumulateClassNames: public std::binary_function<std::set<std::string>*,
    Poco::HashMapEntry<unsigned int, const Object*>&, std::set<std::string>*>
{
public:
  std::set<std::string>* operator()(std::set<std::string>* names,
      Poco::HashMapEntry<unsigned int, const Object*>& entry)
  {
    names->insert(entry.second->GetClassName());
    return names;
  }
};

DebugBreakpointManager* DebugUtil::GetBreakpointManager()
{
  static DebugBreakpointManager breakpointManager;
  return &breakpointManager;
}

void DebugUtil::TraceObject(const Object* object)
{
#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
  CHERRY_INFO << "Tracing enabled for: " << object->GetTraceId() << std::endl;
  m_TracedObjects.insert(object->GetTraceId());
  _G_ObjectEvents.objTracingEvent(object->GetTraceId(), true, object);
#endif
}

void DebugUtil::TraceObject(unsigned int traceId)
{
#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
  CHERRY_INFO << "Tracing enabled for: " << traceId << std::endl;
  m_TracedObjects.insert(traceId);
  TraceIdToObjectType::ConstIterator i = m_TraceIdToObjectMap.find(traceId);
  if (i != m_TraceIdToObjectMap.end())
  _G_ObjectEvents.objTracingEvent(traceId, true, i->second);
  else
  _G_ObjectEvents.objTracingEvent(traceId, true, 0);
#endif
}

void DebugUtil::TraceClass(const std::string& className)
{
#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
  CHERRY_INFO << "Tracing enabled for: " << className << std::endl;
  m_TracedClasses.insert(className);
  //_G_ObjectEvents.objTracingEvent(object->GetTraceId(), true, object);
#endif
}

void DebugUtil::StopTracing(unsigned int traceId)
{
#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
  CHERRY_INFO << "Tracing stopped for: " << traceId << std::endl;
  m_TracedObjects.erase(traceId);
  TraceIdToObjectType::ConstIterator i = m_TraceIdToObjectMap.find(traceId);
  if (i != m_TraceIdToObjectMap.end())
  _G_ObjectEvents.objTracingEvent(traceId, false, i->second);
  else
  _G_ObjectEvents.objTracingEvent(traceId, false, 0);
#endif
}

void DebugUtil::StopTracing(const Object* obj)
{
#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
  CHERRY_INFO << "Tracing stopped for: " << obj->GetTraceId() << std::endl;
  m_TracedObjects.erase(obj->GetTraceId());
  _G_ObjectEvents.objTracingEvent(obj->GetTraceId(), false, obj);
#endif
}

void DebugUtil::StopTracing(const std::string& className)
{
#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
  CHERRY_INFO << "Tracing stopped for: " << className << std::endl;
  m_TracedClasses.erase(className);
  //_G_ObjectEvents.objTracingEvent(obj->GetTraceId(), false, obj);
#endif
}

bool DebugUtil::IsTraced(const Object* object)
{
#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
  return m_TracedObjects.find(object->GetTraceId()) != m_TracedObjects.end();
#else
  return false;
#endif
}

bool DebugUtil::IsTraced(unsigned int traceId)
{
#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
  return m_TracedObjects.find(traceId) != m_TracedObjects.end();
#else
  return false;
#endif
}

bool DebugUtil::IsTraced(const std::string& className)
{
#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
  return m_TracedClasses.find(className) != m_TracedClasses.end();
#else
  return false;
#endif
}

const std::set<unsigned int>& DebugUtil::GetTracedObjects()
{
  return m_TracedObjects;
}

const Object* DebugUtil::GetObject(unsigned int traceId)
{
  return m_TraceIdToObjectMap[traceId];
}

std::list<unsigned int> DebugUtil::GetSmartPointerIDs(
    const Object* objectPointer, const std::list<unsigned int>& excludeList)
{
  poco_assert(objectPointer != 0)
;#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
  std::list<unsigned int> ids = m_TraceIdToSmartPointerMap[objectPointer->GetTraceId()];
  for (std::list<unsigned int>::const_iterator iter = excludeList.begin();
      iter != excludeList.end(); ++iter)
  ids.remove(*iter);
  return ids;
#else
  return std::list<unsigned int>();
#endif
}

void DebugUtil::GetRegisteredObjects(std::vector<const Object*>& list)
{
  for (TraceIdToObjectType::ConstIterator i = m_TraceIdToObjectMap.begin();
      i != m_TraceIdToObjectMap.end(); ++i)
  {
    list.push_back(i->second);
  }
}

void DebugUtil::PrintSmartPointerIDs(const Object* objectPointer, std::ostream& stream, const std::list<unsigned int>& excludeList)
{
  stream << "SmartPointer IDs [ ";
  std::list<unsigned int> ids = GetSmartPointerIDs(objectPointer, excludeList);
  for (std::list<unsigned int>::const_iterator iter = ids.begin();
      iter != ids.end(); ++iter)
  {
    stream << *iter << " ";
  }
  stream << "]\n";
}

void DebugUtil::AddObjectListener(SmartPointer<IDebugObjectListener> listener)
{
  _G_ObjectEvents.AddListener(listener);
}

void DebugUtil::RemoveObjectListener(SmartPointer<IDebugObjectListener> listener)
{
  _G_ObjectEvents.RemoveListener(listener);
}

void DebugUtil::ResetObjectSummary()
{
  m_TraceIdToObjectMap.clear();
  m_TraceIdToSmartPointerMap.clear();
  m_TracedObjects.clear();
  m_TracedClasses.clear();
}

bool DebugUtil::PrintObjectSummary(bool details)
{
  std::set<std::string> names;
  std::accumulate(m_TraceIdToObjectMap.begin(), m_TraceIdToObjectMap.end(), &names, AccumulateClassNames());

  if (!names.empty())
  {
    std::cout << std::endl << std::endl << "#########################################################" << std::endl;
    std::cout << "########     cherry::Object leakage summary:     ########" << std::endl << std::endl;

    for (std::set<std::string>::const_iterator i = names.begin();
        i != names.end(); ++i)
    {
      PrintObjectSummary(*i, details);
      if (details) std::cout << std::endl;
    }

    std::cout << std::endl << "#########################################################" << std::endl << std::endl;
  }

  return !names.empty();
}

bool DebugUtil::PrintObjectSummary(const std::string& className, bool details)
{
  TraceIdToObjectType::ConstIterator endIter =
  std::remove_if(m_TraceIdToObjectMap.begin(), m_TraceIdToObjectMap.end(), NotClassName(className));

  std::cout << "Class: " << className;
  if (details) std::cout << std::endl;

  std::size_t count = 0;
  for (TraceIdToObjectType::ConstIterator iter = m_TraceIdToObjectMap.begin();
      iter != endIter; ++iter, ++count)
  {
    if (details)
    {
      std::cout << *(iter->second);
    }
  }
  std::cout << " (" << count << " instances)" << std::endl;
  return (count!=0);
}

unsigned int& DebugUtil::GetSmartPointerCounter()
{
  static unsigned int counter = 0;
  return counter;
}

void DebugUtil::UnregisterSmartPointer(unsigned int smartPointerId, const Object* objectPointer)
{
#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
  poco_assert(objectPointer != 0);

  m_TraceIdToSmartPointerMap[objectPointer->GetTraceId()].remove(smartPointerId);
  _G_ObjectEvents.spDestroyedEvent(smartPointerId, objectPointer);
#endif
}

void DebugUtil::RegisterSmartPointer(unsigned int smartPointerId, const Object* objectPointer, bool /*recordStack*/)
{
#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
  poco_assert(objectPointer != 0);

  if (m_TracedClasses.find(objectPointer->GetClassName()) != m_TracedClasses.end() ||
      m_TracedObjects.find(objectPointer->GetTraceId()) != m_TracedObjects.end())
  {
    m_TraceIdToSmartPointerMap[objectPointer->GetTraceId()].push_back(smartPointerId);
    _G_ObjectEvents.spCreatedEvent(smartPointerId, objectPointer);
  }

  if (GetBreakpointManager()->BreakAtSmartpointer(smartPointerId))
  poco_debugger_msg("SmartPointer Breakpoint reached");
#endif
}

void DebugUtil::RegisterObject(const Object* objectPointer)
{
#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
  m_TraceIdToObjectMap.insert(std::make_pair(objectPointer->GetTraceId(), objectPointer));
  _G_ObjectEvents.objCreatedEvent(objectPointer);

  if (GetBreakpointManager()->BreakAtObject(objectPointer->GetTraceId()))
  poco_debugger_msg("SmartPointer Breakpoint reached");
#endif
}

void DebugUtil::UnregisterObject(const Object* objectPointer)
{
#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
  m_TraceIdToObjectMap.erase(objectPointer->GetTraceId());
  _G_ObjectEvents.objDestroyedEvent(objectPointer);
#endif
}

bool DebugUtil::GetPersistencePath(Poco::Path& path)
{
  return Platform::GetStatePath(path, Platform::GetBundle("system.bundle"));
}

void DebugUtil::SaveState()
{
  Poco::Path path;
  if (!GetPersistencePath(path)) return;

  path.setFileName(DEBUG_UTIL_XML);

  Poco::XML::Document* doc = new Poco::XML::Document();
  Poco::XML::Element* debugutil = doc->createElement(DEBUGUTIL_TAG);
  doc->appendChild(debugutil)->release();

  for (std::set<unsigned int>::const_iterator i = m_TracedObjects.begin();
      i != m_TracedObjects.end(); ++i)
  {
    Poco::XML::Element* traceObject = doc->createElement(TRACEOBJECT_TAG);
    debugutil->appendChild(traceObject)->release();
    std::stringstream ss;
    ss << *i;
    traceObject->setAttribute(ID_ATTR, ss.str());
  }

  for (std::set<std::string>::const_iterator i = m_TracedClasses.begin();
      i != m_TracedClasses.end(); ++i)
  {
    Poco::XML::Element* traceClass = doc->createElement(TRACECLASS_TAG);
    debugutil->appendChild(traceClass)->release();
    traceClass->setAttribute(NAME_ATTR, *i);
  }

  try
  {
    Poco::FileOutputStream writer(path.toString());
    Poco::XML::DOMWriter out;
    out.setOptions(3); //write declaration and pretty print
    out.writeNode(writer, doc);

    doc->release();

    // save BreakpointManager
    path.setFileName(DebugBreakpointManager::BREAKPOINTS_XML);
    GetBreakpointManager()->SaveState(path);
  }
  catch (Poco::FileException& e)
  {
    CHERRY_WARN << e.displayText();
  }

}

void DebugUtil::RestoreState()
{
  Poco::Path path;
  if (!GetPersistencePath(path)) return;

  path.setFileName(DEBUG_UTIL_XML);

  try
  {
    Poco::XML::DOMParser parser;

    Poco::FileInputStream reader(path.toString());
    Poco::XML::InputSource source(reader);

    //source.setSystemId(baseDir);
    Poco::XML::Document* doc = parser.parse(&source);
    Poco::XML::Element* debugutil = doc->documentElement();

    if (debugutil)
    {
      // restore traced objects
      Poco::XML::NodeList* elementList = debugutil->getElementsByTagName(TRACEOBJECT_TAG);
      for (std::size_t i = 0; i < elementList->length(); i++)
      {
        Poco::XML::Element* elem =
        dynamic_cast<Poco::XML::Element*> (elementList->item(i));

        if (!elem->hasAttribute(ID_ATTR)) continue;

        const std::string& attr = elem->getAttribute(ID_ATTR);

        int traceId = 0;
        try
        {
          traceId = Poco::NumberParser::parse(attr);
        }
        catch (const Poco::SyntaxException& e)
        {
          CHERRY_WARN << e.displayText();
        }

        DebugUtil::TraceObject(traceId);
      }
      elementList->release();

      // restore traced classes
      elementList = debugutil->getElementsByTagName(TRACECLASS_TAG);
      for (std::size_t i = 0; i < elementList->length(); i++)
      {
        Poco::XML::Element* elem =
        dynamic_cast<Poco::XML::Element*> (elementList->item(i));

        if (!elem->hasAttribute(NAME_ATTR)) continue;

        const std::string& traceClass = elem->getAttribute(NAME_ATTR);
        if (!traceClass.empty())
        DebugUtil::TraceClass(traceClass);
      }
      elementList->release();

      debugutil->release();
    }

    doc->release();
  }
  catch (Poco::XML::SAXParseException& e)
  {
    CHERRY_WARN << e.displayText();
  }

  // restore BreakpointManager
  path.setFileName(DebugBreakpointManager::BREAKPOINTS_XML);
  GetBreakpointManager()->RestoreState(path);
}

}
