/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYCOMBINEDEVENTDELTA_H
#define BERRYCOMBINEDEVENTDELTA_H

#include <berrySmartPointer.h>

#include <QExplicitlySharedDataPointer>

namespace berry {

struct CombinedEventDeltaData;
struct IExtension;
struct IExtensionPoint;
struct IExtensionPointFilter;
struct IObjectManager;

class ExtensionPoint;

/**
 * The class stores extensions and extensions points that have been actually
 * modified by a registry operation.
 *
 * For performance, modified extensions and extension points are stored in two forms:
 * - organized in buckets by IDs of extension points (for listeners on specific ext.point)
 * - aggregated in one list (for global listeners)
 */
class CombinedEventDelta
{

public:

  CombinedEventDelta();

  CombinedEventDelta(const CombinedEventDelta& other);
  ~CombinedEventDelta();

  CombinedEventDelta& operator=(const CombinedEventDelta& other);

  void Reset();

  bool IsNull() const;

  static CombinedEventDelta RecordAddition();
  static CombinedEventDelta RecordRemoval();

  bool IsAddition() const;
  bool IsRemoval() const;

  void SetObjectManager(const SmartPointer<IObjectManager> &manager);
  SmartPointer<IObjectManager> GetObjectManager() const;

  void RememberExtensionPoint(const SmartPointer<ExtensionPoint>& extensionPoint);
  void RememberExtension(const SmartPointer<ExtensionPoint>& extensionPoint, int ext);
  void RememberExtensions(const SmartPointer<ExtensionPoint>& extensionPoint,
                          const QList<int>& exts);

  QList<SmartPointer<IExtensionPoint> > GetExtensionPoints(const IExtensionPointFilter& id) const;
  QList<SmartPointer<IExtension> > GetExtensions(const IExtensionPointFilter& id) const;

private:

  CombinedEventDelta(bool addition);

  QList<int> FilterExtensionPoints(const IExtensionPointFilter& filter) const;
  QList<int> FilterExtensions(const IExtensionPointFilter& filter) const;

  QExplicitlySharedDataPointer<CombinedEventDeltaData> d;
};

}

#endif // BERRYCOMBINEDEVENTDELTA_H
