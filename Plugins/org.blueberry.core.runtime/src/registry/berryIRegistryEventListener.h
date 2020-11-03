/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIREGISTRYEVENTLISTENER_H
#define BERRYIREGISTRYEVENTLISTENER_H

#include <berryMessage.h>
#include <berrySmartPointer.h>

#include <QList>

#include <org_blueberry_core_runtime_Export.h>

namespace berry {

struct IExtension;
struct IExtensionPoint;

/**
 * A registry event listener is notified of changes to extension points. Changes
 * include modifications of extension points and their extensions. Listeners will
 * only receive a notification if the extension point they are registered for is
 * modified. (Which includes modifications of extensions under the extension point.)
 * <p>
 * @see IExtensionRegistry#AddListener(IRegistryEventListener, QString)
 */
struct org_blueberry_core_runtime_EXPORT IRegistryEventListener
{

  struct Events {

    Message1<const QList<SmartPointer<IExtension> >&> extensionsAdded;
    Message1<const QList<SmartPointer<IExtension> >&> extensionsRemoved;
    Message1<const QList<SmartPointer<IExtensionPoint> >&> extensionPointsAdded;
    Message1<const QList<SmartPointer<IExtensionPoint> >&> extensionPointsRemoved;

    void AddListener(IRegistryEventListener* listener);
    void RemoveListener(IRegistryEventListener* listener);

  private:

    typedef MessageDelegate1<IRegistryEventListener, const QList<SmartPointer<IExtension> >&> ExtensionsAddedDelegate;
    typedef MessageDelegate1<IRegistryEventListener, const QList<SmartPointer<IExtension> >&> ExtensionsRemovedDelegate;
    typedef MessageDelegate1<IRegistryEventListener, const QList<SmartPointer<IExtensionPoint> >&> ExtensionPointsAddedDelegate;
    typedef MessageDelegate1<IRegistryEventListener, const QList<SmartPointer<IExtensionPoint> >&> ExtensionPointsRemovedDelegate;
  };

  virtual ~IRegistryEventListener();

  /**
   * Notifies this listener that extensions were added to the registry.
   * <p>
   * The extensions supplied as the argument are valid only for the duration of the
   * invocation of this method.
   * </p>
   * @param extensions extensions added to the registry
   */
  virtual void Added(const QList<SmartPointer<IExtension> >& extensions) = 0;

  /**
   * Notifies this listener that extensions were removed from the registry.
   * <p>
   * The extensions supplied as the argument are valid only for the duration of the
   * invocation of this method.
   * </p>
   * @param extensions extensions removed from the registry
   */
  virtual void Removed(const QList<SmartPointer<IExtension> >& extensions) = 0;

  /**
   * Notifies this listener that extension points were added to the registry.
   * <p>
   * The extension points supplied as the argument are valid only for the duration of the
   * invocation of this method.
   * </p>
   * @param extensionPoints extension points added to the registry
   */
  virtual void Added(const QList<SmartPointer<IExtensionPoint> >& extensionPoints) = 0;

  /**
   * Notifies this listener that extension points were removed from the registry.
   * <p>
   * The extension points supplied as the argument are valid only for the duration of the
   * invocation of this method.
   * </p>
   * @param extensionPoints extension points removed from the registry
   */
  virtual void Removed(const QList<SmartPointer<IExtensionPoint> >& extensionPoints) = 0;

};

}

#endif // BERRYIREGISTRYEVENTLISTENER_H
