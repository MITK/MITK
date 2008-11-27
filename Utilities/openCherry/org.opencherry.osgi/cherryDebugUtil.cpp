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

#include "cherryDebugUtil.h"

#include "cherryObject.h"
#include <Poco/Bugcheck.h>

namespace cherry
{

std::map<unsigned long, std::list<int> > DebugUtil::m_TraceIdToSmartPointerMap;
std::set<unsigned long> DebugUtil::m_TracedObjects;

void DebugUtil::TraceObject(const Object* object)
{
#ifdef CHERRY_DEBUG_SMARTPOINTER
  std::cout << "Tracing enabled for: " << object->GetTraceId() << std::endl;
  m_TracedObjects.insert(object->GetTraceId());
#endif
}

bool DebugUtil::IsTraced(const Object* object)
{
#ifdef CHERRY_DEBUG_SMARTPOINTER
  return m_TracedObjects.find(object->GetTraceId()) != m_TracedObjects.end();
#else
  return false;
#endif
}

std::list<int> DebugUtil::GetSmartPointerIDs(const Object* objectPointer, const std::list<int>& excludeList)
{
  poco_assert(objectPointer != 0);

#ifdef CHERRY_DEBUG_SMARTPOINTER
  std::list<int> ids = m_TraceIdToSmartPointerMap[objectPointer->GetTraceId()];
  for (std::list<int>::const_iterator iter = excludeList.begin();
       iter != excludeList.end(); ++iter)
    ids.remove(*iter);
  return ids;
#else
  return std::list<int>();
#endif
}

void DebugUtil::PrintSmartPointerIDs(const Object* objectPointer, std::ostream& stream, const std::list<int>& excludeList)
{
  stream << "SmartPointer IDs [ ";
  std::list<int> ids = GetSmartPointerIDs(objectPointer, excludeList);
  for (std::list<int>::const_iterator iter = ids.begin();
       iter != ids.end(); ++iter)
  {
    stream << *iter << " ";
  }
  stream << "]\n";
}

int& DebugUtil::GetSmartPointerCounter()
{
  static int counter = 0;
  return counter;
}

void DebugUtil::UnregisterSmartPointer(int smartPointerId, const Object* objectPointer)
{
#ifdef CHERRY_DEBUG_SMARTPOINTER
  if (objectPointer != 0)
  {
    m_TraceIdToSmartPointerMap[objectPointer->GetTraceId()].remove(smartPointerId);
  }
#endif
}

void DebugUtil::RegisterSmartPointer(int smartPointerId, const Object* objectPointer, bool /*recordStack*/)
{
  poco_assert(objectPointer != 0);

#ifdef CHERRY_DEBUG_SMARTPOINTER
  m_TraceIdToSmartPointerMap[objectPointer->GetTraceId()].push_back(smartPointerId);
#endif
}

}
