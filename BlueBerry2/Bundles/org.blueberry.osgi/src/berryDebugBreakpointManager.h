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


#ifndef BERRYDEBUGBREAKPOINTMANAGER_H_
#define BERRYDEBUGBREAKPOINTMANAGER_H_

#include <Poco/Path.h>
#include <Poco/HashMap.h>
#include <set>

#include "berryOSGiDll.h"

namespace berry {

class Object;

class BERRY_OSGI DebugBreakpointManager
{
public:

  static const std::string BREAKPOINTS_XML;

  void AddSmartpointerBreakpoint(int smartPointerId, const Object* obj = 0);
  void AddObjectBreakpoint(unsigned long objectTraceId);

  void RemoveSmartpointerBreakpoint(int smartPointerId);
  void RemoveObjectBreakpoint(unsigned long objectTraceId);

  const std::set<unsigned long>& GetObjectBreakpoints() const;
  const Poco::HashMap<int, const Object*>& GetSmartPointerBreakpoints() const;

  bool BreakAtObject(unsigned long traceId) const;
  bool BreakAtSmartpointer(int spId) const;

  void SaveState(const Poco::Path& path) const;
  void RestoreState(const Poco::Path& path);

private:

  friend class DebugUtil;

  DebugBreakpointManager() {}
  DebugBreakpointManager(const DebugBreakpointManager&) {}

  static const std::string BREAKPOINTS_TAG;
  static const std::string OBJECT_TAG;
  static const std::string SMARTPOINTER_TAG;

  static const std::string ID_ATTR;
  static const std::string CLASSNAME_ATTR;
  static const std::string OBJECTID_ATTR;
  static const std::string ENABLED_ATTR;

  std::set<unsigned long> m_ObjectBreakpoints;
  Poco::HashMap<int, const Object*> m_SmartPointerBreakpoints;
};

}

#endif /* BERRYDEBUGBREAKPOINTMANAGER_H_ */
