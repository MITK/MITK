/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYEXTENSIONTRACKER_H
#define BERRYEXTENSIONTRACKER_H

#include <berryIExtensionTracker.h>

namespace berry {

struct IExtension;
struct IExtensionPoint;
struct IExtensionRegistry;
struct IExtensionPointFilter;

/**
 * Implementation of the IExtensionTracker.
 *
 * @see IExtensionTracker
 */
class org_blueberry_core_runtime_EXPORT ExtensionTracker : public IExtensionTracker
{
  struct Impl;
  QScopedPointer<Impl> d;

public:

  ExtensionTracker();
  ~ExtensionTracker() override;

  /**
   * Construct a new instance of the extension tracker using the given registry
   * containing tracked extensions and extension points.
   *
   * @param theRegistry the extension registry to track
   */
  ExtensionTracker(IExtensionRegistry* theRegistry);

  void RegisterHandler(IExtensionChangeHandler* handler, const IExtensionPointFilter& filter) override;

  void RegisterHandler(IExtensionChangeHandler* handler, const QString& extensionPointId) override;

  void UnregisterHandler(IExtensionChangeHandler* handler) override;

  void RegisterObject(const SmartPointer<IExtension>& element,
                      const SmartPointer<Object>& object, ReferenceType referenceType) override;

  QList<SmartPointer<Object> > GetObjects(const SmartPointer<IExtension>& element) const override;

  void Close() override;

  void UnregisterObject(const SmartPointer<IExtension>& extension, const SmartPointer<Object>& object) override;

  QList<SmartPointer<Object> > UnregisterObject(const SmartPointer<IExtension>& extension) override;

  /**
   * Return an instance of filter matching all changes for the given extension point.
   *
   * @param xpt the extension point
   * @return a filter
   */
  static IExtensionPointFilter CreateExtensionPointFilter(const SmartPointer<IExtensionPoint>& xpt);

  /**
   * Return an instance of filter matching all changes for the given extension points.
   *
   * @param xpts the extension points used to filter
   * @return a filter
   */
  static IExtensionPointFilter CreateExtensionPointFilter(const QList<SmartPointer<IExtensionPoint> >& xpts);

  /**
   * Return an instance of filter matching all changes from a given plugin.
   *
   * @param id the plugin id
   * @return a filter
   */
  static IExtensionPointFilter CreateNamespaceFilter(const QString& id);


protected:

  virtual void ApplyAdd(IExtensionChangeHandler* handler, const SmartPointer<IExtension>& extension);

  virtual void ApplyRemove(IExtensionChangeHandler* handler, const SmartPointer<IExtension>& removedExtension,
                           const QList<SmartPointer<Object> >& removedObjects);

private:

  void Init(IExtensionRegistry* registry);
};

}

#endif // BERRYEXTENSIONTRACKER_H
