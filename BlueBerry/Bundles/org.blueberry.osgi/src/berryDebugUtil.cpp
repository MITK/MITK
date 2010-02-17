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

#include "berryIDebugObjectListener.h"

#include "berryDebugUtil.h"
#include "berryObject.h"
#include "berryLog.h"
#include "berryPlatform.h"
#include "berryDebugBreakpointManager.h"

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

namespace berry
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

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
void DebugUtil::TraceObject(const Object* object)
{
  BERRY_INFO << "Tracing enabled for: " << object->GetTraceId() << std::endl;
  m_TracedObjects.insert(object->GetTraceId());
  _G_ObjectEvents.objTracingEvent(object->GetTraceId(), true, object);
}
#else
void DebugUtil::TraceObject(const Object*  /*object*/)
{
}
#endif

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
void DebugUtil::TraceObject(unsigned int traceId)
{
  BERRY_INFO << "Tracing enabled for: " << traceId << std::endl;
  m_TracedObjects.insert(traceId);
  TraceIdToObjectType::ConstIterator i = m_TraceIdToObjectMap.find(traceId);
  if (i != m_TraceIdToObjectMap.end())
  _G_ObjectEvents.objTracingEvent(traceId, true, i->second);
  else
  _G_ObjectEvents.objTracingEvent(traceId, true, 0);
}
#else
void DebugUtil::TraceObject(unsigned int /*traceId*/)
{
}
#endif

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
void DebugUtil::TraceClass(const std::string& className)
{
  BERRY_INFO << "Tracing enabled for: " << className << std::endl;
  m_TracedClasses.insert(className);
  //_G_ObjectEvents.objTracingEvent(object->GetTraceId(), true, object);
}
#else
void DebugUtil::TraceClass(const std::string&  /*className*/)
{
}
#endif

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
void DebugUtil::StopTracing(unsigned int traceId)
{

  BERRY_INFO << "Tracing stopped for: " << traceId << std::endl;
  m_TracedObjects.erase(traceId);
  TraceIdToObjectType::ConstIterator i = m_TraceIdToObjectMap.find(traceId);
  if (i != m_TraceIdToObjectMap.end())
  _G_ObjectEvents.objTracingEvent(traceId, false, i->second);
  else
  _G_ObjectEvents.objTracingEvent(traceId, false, 0);
}
#else
void DebugUtil::StopTracing(unsigned int /*traceId*/)
{
}
#endif

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
void DebugUtil::StopTracing(const Object* obj)
{
  BERRY_INFO << "Tracing stopped for: " << obj->GetTraceId() << std::endl;
  m_TracedObjects.erase(obj->GetTraceId());
  _G_ObjectEvents.objTracingEvent(obj->GetTraceId(), false, obj);
}
#else
void DebugUtil::StopTracing(const Object* /*obj*/)
{
}
#endif

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
void DebugUtil::StopTracing(const std::string& className)
{
  BERRY_INFO << "Tracing stopped for: " << className << std::endl;
  m_TracedClasses.erase(className);
  //_G_ObjectEvents.objTracingEvent(obj->GetTraceId(), false, obj);
}
#else
void DebugUtil::StopTracing(const std::string& /*className*/)
{
}
#endif

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
bool DebugUtil::IsTraced(const Object* object)
{
  if (m_TracedObjects.find(object->GetTraceId()) != m_TracedObjects.end())
    return true;

  if (m_TracedClasses.find(object->GetClassName()) != m_TracedClasses.end())
      return true;

  return false;
}
#else
bool DebugUtil::IsTraced(const Object*  /*object*/)
{
  return false;
}
#endif

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
bool DebugUtil::IsTraced(unsigned int traceId)
{
  if (m_TracedObjects.find(traceId) != m_TracedObjects.end())
    return true;

  TraceIdToObjectType::Iterator it = m_TraceIdToObjectMap.find(traceId);
  if (it != m_TraceIdToObjectMap.end())
  {
    if (m_TracedClasses.find(it->second->GetClassName()) != m_TracedClasses.end())
      return true;
  }

  return false;
}
#else
bool DebugUtil::IsTraced(unsigned int /*traceId*/)
{
  return false;
}
#endif

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
bool DebugUtil::IsTraced(const std::string& className)
{
  return m_TracedClasses.find(className) != m_TracedClasses.end();
}
#else
bool DebugUtil::IsTraced(const std::string&  /*className*/)
{
  return false;
}
#endif

const std::set<unsigned int>& DebugUtil::GetTracedObjects()
{
  return m_TracedObjects;
}

const Object* DebugUtil::GetObject(unsigned int traceId)
{
  return m_TraceIdToObjectMap[traceId];
}

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
std::list<unsigned int> DebugUtil::GetSmartPointerIDs(
    const Object* objectPointer, const std::list<unsigned int>& excludeList)
{
  poco_assert(objectPointer != 0);
  std::list<unsigned int> ids = m_TraceIdToSmartPointerMap[objectPointer->GetTraceId()];
  for (std::list<unsigned int>::const_iterator iter = excludeList.begin();
      iter != excludeList.end(); ++iter)
  ids.remove(*iter);
  return ids;
}
#else
std::list<unsigned int> DebugUtil::GetSmartPointerIDs(
    const Object* /*objectPointer*/, const std::list<unsigned int>&  /*excludeList*/)
{
  return std::list<unsigned int>();
}
#endif

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
  if (IsTraced(objectPointer))
  {
    std::list<unsigned int> ids = GetSmartPointerIDs(objectPointer, excludeList);
    for (std::list<unsigned int>::const_iterator iter = ids.begin();
        iter != ids.end(); ++iter)
    {
      stream << *iter << " ";
    }
  }
  else
  {
    stream << "n/a ";
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
}

bool DebugUtil::PrintObjectSummary(bool details)
{
  std::set<std::string> names;
  std::accumulate(m_TraceIdToObjectMap.begin(), m_TraceIdToObjectMap.end(), &names, AccumulateClassNames());

  if (!names.empty())
  {
    std::cout << std::endl << std::endl << "#########################################################" << std::endl;
    std::cout << "########     berry::Object leakage summary:     ########" << std::endl << std::endl;

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
      PrintSmartPointerIDs(iter->second, std::cout);
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

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
void DebugUtil::UnregisterSmartPointer(unsigned int smartPointerId, const Object* objectPointer)
{
  poco_assert(objectPointer != 0);

  m_TraceIdToSmartPointerMap[objectPointer->GetTraceId()].remove(smartPointerId);
  _G_ObjectEvents.spDestroyedEvent(smartPointerId, objectPointer);
}
#else
void DebugUtil::UnregisterSmartPointer(unsigned int  /*smartPointerId*/, const Object*  /*objectPointer*/)
{
}
#endif

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
void DebugUtil::RegisterSmartPointer(unsigned int  smartPointerId, const Object* objectPointer, bool recordStack)
{
  poco_assert(objectPointer != 0);

  if (m_TracedClasses.find(objectPointer->GetClassName()) != m_TracedClasses.end() ||
      m_TracedObjects.find(objectPointer->GetTraceId()) != m_TracedObjects.end())
  {
    m_TraceIdToSmartPointerMap[objectPointer->GetTraceId()].push_back(smartPointerId);
    _G_ObjectEvents.spCreatedEvent(smartPointerId, objectPointer);
  }

  if (GetBreakpointManager()->BreakAtSmartpointer(smartPointerId))
  poco_debugger_msg("SmartPointer Breakpoint reached");
}
#else
void DebugUtil::RegisterSmartPointer(unsigned int  /*smartPointerId*/, const Object*  /*objectPointer*/, bool /*recordStack*/)
{
}
#endif

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
void DebugUtil::RegisterObject(const Object* objectPointer)
{
  m_TraceIdToObjectMap.insert(std::make_pair(objectPointer->GetTraceId(), objectPointer));
  _G_ObjectEvents.objCreatedEvent(objectPointer);

  if (GetBreakpointManager()->BreakAtObject(objectPointer->GetTraceId()))
  poco_debugger_msg("SmartPointer Breakpoint reached");
}
#else
void DebugUtil::RegisterObject(const Object* /*objectPointer*/)
{
}
#endif

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
void DebugUtil::UnregisterObject(const Object* objectPointer)
{
  m_TraceIdToObjectMap.erase(objectPointer->GetTraceId());
  _G_ObjectEvents.objDestroyedEvent(objectPointer);
}
#else
void DebugUtil::UnregisterObject(const Object* /*objectPointer*/)
{
}
#endif

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
    BERRY_WARN << e.displayText();
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
        dynamic_cast<Poco::XML::Element*> (elementList->item(static_cast<unsigned long>(i)));

        if (!elem->hasAttribute(ID_ATTR)) continue;

        const std::string& attr = elem->getAttribute(ID_ATTR);

        int traceId = 0;
        try
        {
          traceId = Poco::NumberParser::parse(attr);
        }
        catch (const Poco::SyntaxException& e)
        {
          BERRY_WARN << e.displayText();
        }

        DebugUtil::TraceObject(traceId);
      }
      elementList->release();

      // restore traced classes
      elementList = debugutil->getElementsByTagName(TRACECLASS_TAG);
      for (std::size_t i = 0; i < elementList->length(); i++)
      {
        Poco::XML::Element* elem =
        dynamic_cast<Poco::XML::Element*> (elementList->item(static_cast<unsigned long>(i)));

        if (!elem->hasAttribute(NAME_ATTR)) continue;

        const std::string& traceClass = elem->getAttribute(NAME_ATTR);
        if (!traceClass.empty())
        DebugUtil::TraceClass(traceClass);
      }
      elementList->release();
    }

    doc->release();
  }
  catch (Poco::XML::SAXParseException& e)
  {
    BERRY_WARN << e.displayText();
  }
  catch (Poco::FileNotFoundException&)
  {

  }
  catch (Poco::FileException& e)
  {
    BERRY_WARN << e.displayText();
  }

  // restore BreakpointManager
  path.setFileName(DebugBreakpointManager::BREAKPOINTS_XML);
  GetBreakpointManager()->RestoreState(path);
}

}
