/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYDEBUGBREAKPOINTMANAGER_H_
#define BERRYDEBUGBREAKPOINTMANAGER_H_

#include <Poco/HashMap.h>
#include <set>

#include <org_blueberry_core_runtime_Export.h>

namespace berry {

class Object;

class org_blueberry_core_runtime_EXPORT DebugBreakpointManager
{
public:

  static const std::string BREAKPOINTS_XML;

  void AddSmartpointerBreakpoint(int smartPointerId, const Object* obj = nullptr);
  void AddObjectBreakpoint(unsigned long objectTraceId);

  void RemoveSmartpointerBreakpoint(int smartPointerId);
  void RemoveObjectBreakpoint(unsigned long objectTraceId);

  const std::set<unsigned long>& GetObjectBreakpoints() const;
  const Poco::HashMap<int, const Object*>& GetSmartPointerBreakpoints() const;

  bool BreakAtObject(unsigned long traceId) const;
  bool BreakAtSmartpointer(int spId) const;

  void SaveState(const QString& path) const;
  void RestoreState(const QString& path);

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
