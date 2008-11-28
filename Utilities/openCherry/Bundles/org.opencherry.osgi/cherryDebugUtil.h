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

#include "cherryOSGiDll.h"

namespace cherry {

class Object;

class CHERRY_OSGI DebugUtil
{

public:

  static void TraceObject(const Object*);
  static bool IsTraced(const Object* object);

  static std::list<int> GetSmartPointerIDs(const Object* objectPointer, const std::list<int>& excludeList = std::list<int>());

  static void PrintSmartPointerIDs(const Object* objectPointer, std::ostream& = std::cout, const std::list<int>& excludeList = std::list<int>());


  // ******* for internal use only *************
  static int& GetSmartPointerCounter();
  static void RegisterSmartPointer(int smartPointerId, const Object* objectPointer, bool recordStack = false);
  static void UnregisterSmartPointer(int smartPointerId, const Object* objectPointer);
  // *******************************************

private:

  static std::map<unsigned long, std::list<int> > m_TraceIdToSmartPointerMap;
  static std::set<unsigned long> m_TracedObjects;
};

}


#endif /* CHERRYDEBUGUTIL_H_ */
