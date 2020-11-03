/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*//*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIEXTENSIONTRACKER_H
#define BERRYIEXTENSIONTRACKER_H

#include <org_blueberry_core_runtime_Export.h>

#include "berrySmartPointer.h"

#include <QList>

namespace berry {

struct IExtension;
struct IExtensionChangeHandler;
struct IExtensionPointFilter;

class Object;

/**
 * An extension tracker keeps associations between extensions and their derived
 * objects on an extension basis. All extensions being added in a tracker will
 * automatically be removed when the extension is uninstalled from the registry.
 * Users interested in extension removal can register a handler that will let
 * them know when an object is being removed.
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct org_blueberry_core_runtime_EXPORT IExtensionTracker
{

  enum ReferenceType {
    /**
     * Constant for strong (normal) reference holding.
     */
    REF_STRONG,

    /**
     * Constant for weak reference holding.
     */
    REF_WEAK
  };

  virtual ~IExtensionTracker();

  /**
   * Register an extension change handler with this tracker using the given filter.
   *
   * @param handler the handler to be registered
   * @param filter the filter to use to choose interesting changes
   */
  virtual void RegisterHandler(IExtensionChangeHandler* handler, const IExtensionPointFilter& filter) = 0;

  /**
   * Register an extension change handler with this tracker for the given extension point id.
   *
   * @param handler the handler to be registered
   * @param extensionPointId the extension point id to track
   */
  virtual void RegisterHandler(IExtensionChangeHandler *handler, const QString& extensionPointId = QString()) = 0;

  /**
   * Unregister the given extension change handler previously registered with this tracker.
   *
   * @param handler the handler to be unregistered
   */
  virtual void UnregisterHandler(IExtensionChangeHandler* handler) = 0;

  /**
   * Create an association between the given extension and the given object.
   * The referenceType indicates how strongly the object is being kept in memory.
   * There are 2 possible values: REF_STRONG and REF_WEAK.
   *
   * @param extension the extension
   * @param object the object to associate with the extension
   * @param referenceType one of REF_STRONG, REF_WEAK
   */
  virtual void RegisterObject(const SmartPointer<IExtension>& extension,
                              const SmartPointer<Object>& object, ReferenceType referenceType) = 0;

  /**
   * Remove an association between the given extension and the given object.
   *
   * @param extension the extension under which the object has been registered
   * @param object the object to unregister
   */
  virtual void UnregisterObject(const SmartPointer<IExtension>& extension,
                                const SmartPointer<Object>& object) = 0;

  /**
   * Remove all the objects associated with the given extension. Return
   * the removed objects.
   *
   * @param extension the extension for which the objects are removed
   * @return the objects that were associated with the extension
   */
  virtual QList<SmartPointer<Object> > UnregisterObject(const SmartPointer<IExtension>& extension) = 0;

  /**
   * Return all the objects that have been associated with the given extension.
   * All objects registered strongly will be return unless they have been unregistered.
   * The objects registered softly or weakly may not be returned if they have been garbage collected.
   * Return an empty array if no associations exist.
   *
   * @param extension the extension for which the object must be returned
   * @return the array of associated objects
   */
  virtual QList<SmartPointer<Object> > GetObjects(const SmartPointer<IExtension>& extension) const = 0;

  /**
   * Close the tracker. All registered objects are freed and all handlers are being automatically removed.
   */
  virtual void Close() = 0;
};

}

#endif // BERRYIEXTENSIONTRACKER_H
