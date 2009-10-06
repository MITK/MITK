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


#ifndef CHERRYDEBUGUTIL_H_
#define CHERRYDEBUGUTIL_H_

#include <map>
#include <list>
#include <set>
#include <iostream>

#include <Poco/HashMap.h>
#include <Poco/Path.h>

#include "cherryOSGiDll.h"

namespace cherry {

class Object;
class DebugBreakpointManager;
class IDebugObjectListener;
template <class T> class SmartPointer;

class CHERRY_OSGI DebugUtil
{

public:

  static DebugBreakpointManager* GetBreakpointManager();

  static void TraceObject(const Object*);
  static void TraceObject(unsigned long traceId);
  static void TraceClass(const std::string& className);

  static void StopTracing(unsigned long traceId);
  static void StopTracing(const Object* obj);
  static void StopTracing(const std::string& className);

  static bool IsTraced(const Object* object);
  static bool IsTraced(unsigned long traceId);
  static bool IsTraced(const std::string& className);

  static const std::set<unsigned long>& GetTracedObjects();

  static const Object* GetObject(unsigned long traceId);

  static std::list<int> GetSmartPointerIDs(const Object* objectPointer, const std::list<int>& excludeList = std::list<int>());
  static void GetRegisteredObjects(std::vector<const Object*>& list);

  static void PrintSmartPointerIDs(const Object* objectPointer, std::ostream& = std::cout, const std::list<int>& excludeList = std::list<int>());

  static void ResetObjectSummary();
  static bool PrintObjectSummary(bool details = false);
  static bool PrintObjectSummary(const std::string& className, bool details = false);

  static void AddObjectListener(SmartPointer<IDebugObjectListener> listener);
  static void RemoveObjectListener(SmartPointer<IDebugObjectListener> listener);

  static void SaveState();
  static void RestoreState();

  // ******* for internal use only *************
  static int& GetSmartPointerCounter();
  static void RegisterSmartPointer(int smartPointerId, const Object* objectPointer, bool recordStack = false);
  static void UnregisterSmartPointer(int smartPointerId, const Object* objectPointer);
  static void RegisterObject(const Object* objectPointer);
  static void UnregisterObject(const Object* objectPointer);
  // *******************************************

private:

  static const std::string DEBUG_UTIL_XML;

  static const std::string DEBUGUTIL_TAG;
  static const std::string TRACEOBJECT_TAG;
  static const std::string TRACECLASS_TAG;

  static const std::string ID_ATTR;
  static const std::string NAME_ATTR;

  static bool GetPersistencePath(Poco::Path& path);

  struct ObjectHash : public std::unary_function<const Object*, std::size_t>
  {
    std::size_t operator()(const Object* obj) const
    {
      return reinterpret_cast<std::size_t> (this);
    }
  };

  static Poco::HashMap<unsigned long, std::list<int> > m_TraceIdToSmartPointerMap;
  typedef Poco::HashMap<unsigned long, const Object*> TraceIdToObjectType;
  static TraceIdToObjectType m_TraceIdToObjectMap;
  static std::set<unsigned long> m_TracedObjects;
  static std::set<std::string> m_TracedClasses;
};

}


#endif /* CHERRYDEBUGUTIL_H_ */
