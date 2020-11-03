/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYDEBUGUTIL_H_
#define BERRYDEBUGUTIL_H_

#include <QHash>
#include <QSet>
#include <QList>

#include <org_blueberry_core_runtime_Export.h>

class QDir;

namespace berry {

class Object;
class DebugBreakpointManager;
struct IDebugObjectListener;
template <class T> class SmartPointer;

class org_blueberry_core_runtime_EXPORT DebugUtil
{

public:

  static DebugBreakpointManager* GetBreakpointManager();

  static void TraceObject(const Object*);
  static void TraceObject(unsigned int traceId);
  static void TraceClass(const QString& className);

  static void StopTracing(unsigned int traceId);
  static void StopTracing(const Object* obj);
  static void StopTracing(const QString& className);

  static bool IsTraced(const Object* object);
  static bool IsTraced(unsigned int traceId);
  static bool IsTraced(const QString& className);

  static QSet<unsigned int> GetTracedObjects();

  static const Object* GetObject(unsigned int traceId);

  static QList<unsigned int> GetSmartPointerIDs(const Object* objectPointer, const QList<unsigned int>& excludeList = QList<unsigned int>());
  static QList<const Object*> GetRegisteredObjects();

  static void PrintSmartPointerIDs(const Object* objectPointer, const QList<unsigned int>& excludeList = QList<unsigned int>());

  static void ResetObjectSummary();
  static bool PrintObjectSummary(bool details = false);
  static bool PrintObjectSummary(const QString& className, bool details = false);

  static void AddObjectListener(IDebugObjectListener* listener);
  static void RemoveObjectListener(IDebugObjectListener* listener);

  static void SaveState(const QDir& path);
  static void RestoreState(const QDir& path);

  // ******* for internal use only *************
  static unsigned int& GetSmartPointerCounter();
  static void RegisterSmartPointer(unsigned int smartPointerId, const Object* objectPointer, bool recordStack = false);
  static void UnregisterSmartPointer(unsigned int smartPointerId, const Object* objectPointer);
  static void RegisterObject(const Object* objectPointer);
  static void UnregisterObject(const Object* objectPointer);
  // *******************************************

private:

  static const QString DEBUG_UTIL_XML;

  static const QString DEBUGUTIL_TAG;
  static const QString TRACEOBJECT_TAG;
  static const QString TRACECLASS_TAG;

  static const QString ID_ATTR;
  static const QString NAME_ATTR;

  static bool GetPersistencePath(QDir& path);

  static QHash<quint32, QList<unsigned int> > m_TraceIdToSmartPointerMap;
  typedef QHash<quint32, const Object* > TraceIdToObjectType;
  static TraceIdToObjectType m_TraceIdToObjectMap;
  static QSet<unsigned int> m_TracedObjects;
  static QSet<QString> m_TracedClasses;
};

}


#endif /* BERRYDEBUGUTIL_H_ */
