/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryCombinedEventDelta.h"
#include "berryExtensionPoint.h"
#include "berryExtensionPointHandle.h"
#include "berryIExtensionPoint.h"
#include "berryExtensionHandle.h"
#include "berryIExtension.h"
#include "berryIExtensionPointFilter.h"
#include "berryIObjectManager.h"

#include "berrySimpleExtensionPointFilter.h"

#include <QSharedData>

namespace berry {

struct CombinedEventDeltaData : public QSharedData
{
  const bool addition; // true: objects were added; false: objects were removed

  // the object manager from which all the objects contained in this delta will be found
  SmartPointer<IObjectManager> objectManager;

  QHash<QString, QList<int> > extensionsByID; // extension point ID -> List of Integer extensions IDs
  QHash<QString, QList<int> > extPointsByID; // extension point ID -> List of Integer extension point IDs

  QList<int> allExtensions; // List of Integer IDs
  QList<int> allExtensionPoints; // List if Integer IDs

  CombinedEventDeltaData(bool addition)
    : addition(addition), objectManager(nullptr)
  {
  }

  QList<int>& GetExtensionsBucket(const QString& id)
  {
    return extensionsByID[id];
  }

  QList<int>& GetExtPointsBucket(const QString& id)
  {
    return extPointsByID[id];
  }

  QList<int>& GetExtPointsGlobal()
  {
    return allExtensionPoints;
  }

  QList<int>& GetExtensionsGlobal()
  {
    return allExtensions;
  }
};

CombinedEventDelta::CombinedEventDelta(bool addition)
  : d(new CombinedEventDeltaData(addition))
{
}

QList<int> CombinedEventDelta::FilterExtensions(const IExtensionPointFilter& filter) const
{
  if (const SimpleExtensionPointFilter* simpleFilter = dynamic_cast<const SimpleExtensionPointFilter*>(filter.GetConcept()))
  {
    if (simpleFilter->m_Id.isEmpty()) return d->allExtensions;
    return d->extensionsByID[simpleFilter->m_Id];
  }

  QList<int> result;
  foreach(int extPt, d->allExtensionPoints)
  {
    ExtensionPointHandle handle(d->objectManager, extPt);
    if (filter.Matches(&handle))
    {
      result.append(d->extensionsByID[handle.GetUniqueIdentifier()]);
    }
  }
  return result;
}

QList<int> CombinedEventDelta::FilterExtensionPoints(const IExtensionPointFilter& filter) const
{
  if (const SimpleExtensionPointFilter* simpleFilter = dynamic_cast<const SimpleExtensionPointFilter*>(filter.GetConcept()))
  {
    if (simpleFilter->m_Id.isEmpty()) return d->allExtensionPoints;
    return d->extPointsByID[simpleFilter->m_Id];
  }

  QList<int> result;
  foreach(int extPt, d->allExtensionPoints)
  {
    ExtensionPointHandle handle(d->objectManager, extPt);
    if (filter.Matches(&handle))
    {
      result.push_back(extPt);
    }
  }
  return result;
}

CombinedEventDelta::CombinedEventDelta()
{
}

CombinedEventDelta::CombinedEventDelta(const CombinedEventDelta &other)
  : d(other.d)
{
}

CombinedEventDelta::~CombinedEventDelta()
{

}

CombinedEventDelta &CombinedEventDelta::operator =(const CombinedEventDelta &other)
{
  d = other.d;
  return *this;
}

void CombinedEventDelta::Reset()
{
  d.reset();
}

bool CombinedEventDelta::IsNull() const
{
  return !d;
}

CombinedEventDelta CombinedEventDelta::RecordAddition()
{
  return CombinedEventDelta(true);
}

CombinedEventDelta CombinedEventDelta::RecordRemoval()
{
  return CombinedEventDelta(false);
}

bool CombinedEventDelta::IsAddition() const
{
  return d->addition;
}

bool CombinedEventDelta::IsRemoval() const
{
  return !d->addition;
}

void CombinedEventDelta::SetObjectManager(const SmartPointer<IObjectManager>& manager)
{
  d->objectManager = manager;
}

SmartPointer<IObjectManager> CombinedEventDelta::GetObjectManager() const
{
  return d->objectManager;
}

void CombinedEventDelta::RememberExtensionPoint(const SmartPointer<ExtensionPoint>& extensionPoint)
{
  QString bucketId = extensionPoint->GetUniqueIdentifier();
  int extPt = extensionPoint->GetObjectId();
  d->GetExtPointsBucket(bucketId).push_back(extPt);
  d->GetExtPointsGlobal().push_back(extPt);
}

void CombinedEventDelta::RememberExtension(const SmartPointer<ExtensionPoint>& extensionPoint, int ext)
{
  QString bucketId = extensionPoint->GetUniqueIdentifier();

  d->GetExtensionsBucket(bucketId).push_back(ext);
  d->GetExtensionsGlobal().push_back(ext);
}

void CombinedEventDelta::RememberExtensions(const SmartPointer<ExtensionPoint>& extensionPoint,
                                            const QList<int>& exts)
{
  for (int i = 0; i < exts.size(); ++i)
    RememberExtension(extensionPoint, exts[i]);
}

QList<SmartPointer<IExtensionPoint> > CombinedEventDelta::GetExtensionPoints(const IExtensionPointFilter& filter) const
{
  QList<int> extensionPoints;
  if (!filter.IsNull() && !d->extPointsByID.isEmpty())
  {
    extensionPoints = FilterExtensionPoints(filter);
  }
  else if (filter.IsNull())
  {
    extensionPoints = d->allExtensionPoints;
  }

  if (extensionPoints.isEmpty()) // no changes that fit the filter
    return QList<IExtensionPoint::Pointer>();

  QList<IExtensionPoint::Pointer> result;
  for (int i = 0; i < extensionPoints.size(); ++i)
  {
    int extPt = extensionPoints[i];
    IExtensionPoint::Pointer extensionPoint(new ExtensionPointHandle(d->objectManager, extPt));
    result.push_back(extensionPoint);
  }
  return result;
}

QList<SmartPointer<IExtension> > CombinedEventDelta::GetExtensions(const IExtensionPointFilter& filter) const
{
  QList<int> extensions;
  if (!filter.IsNull() && !d->extensionsByID.isEmpty())
  {
    extensions = FilterExtensions(filter);
  }
  else if (filter.IsNull())
  {
    extensions = d->allExtensions;
  }

  if (extensions.isEmpty()) // no changes that fit the filter
    return QList<IExtension::Pointer>();

  QList<IExtension::Pointer> result;
  for (int i = 0; i < extensions.size(); ++i)
  {
    int ext = extensions[i];
    IExtension::Pointer extension(new ExtensionHandle(d->objectManager, ext));
    result.push_back(extension);
  }
  return result;
}

}
