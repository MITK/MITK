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

#ifndef BERRYIEXTENSIONCHANGEHANDLER_H
#define BERRYIEXTENSIONCHANGEHANDLER_H

#include <berrySmartPointer.h>

#include <org_blueberry_core_runtime_Export.h>

namespace berry {

struct IExtension;
struct IExtensionTracker;

class Object;

/**
 * Extension change handlers are notified of changes for a given extension
 * point in the context of an extension tracker.
 * <p>
 * This interface can be used without OSGi running.
 * </p><p>
 * This interface is intended to be implemented by clients.
 * </p>
 */
struct org_blueberry_core_runtime_EXPORT IExtensionChangeHandler
{
  virtual ~IExtensionChangeHandler();

  /**
   * This method is called whenever an extension conforming to the extension point filter
   * is being added to the registry. This method does not automatically register objects
   * to the tracker.
   *
   * @param tracker a tracker to which the handler has been registered
   * @param extension the extension being added
   */
  virtual void AddExtension(IExtensionTracker* tracker, const SmartPointer<IExtension>& extension) = 0;

  /**
   * This method is called after the removal of an extension.
   *
   * @param extension the extension being removed
   * @param objects the objects that were associated with the removed extension
   */
  virtual void RemoveExtension(const SmartPointer<IExtension>& extension,
                               const QList<SmartPointer<Object> >& objects) = 0;
};


}

#endif // BERRYIEXTENSIONCHANGEHANDLER_H
