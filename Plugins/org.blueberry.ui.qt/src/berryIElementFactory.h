/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIELEMENTFACTORY_H
#define BERRYIELEMENTFACTORY_H

#include <org_blueberry_ui_qt_Export.h>

#include <berrySmartPointer.h>

namespace berry {

struct IAdaptable;
struct IMemento;

/**
 * A factory for re-creating objects from a previously saved memento.
 * <p>
 * Clients should implement this interface and include the name of their class
 * in an extension to the platform extension point named
 * <code>"org.blueberry.ui.elementFactories"</code>.
 * For example, the plug-in's XML markup might contain:
 * <pre>
 * &LT;extension point="org.blueberry.ui.elementFactories"&GT;
 *    &LT;factory id="com.example.myplugin.MyFactory" class="MyFactory" /&GT;
 * &LT;/extension&GT;
 * </pre>
 * </p>
 *
 * @see IPersistableElement
 * @see IMemento
 * @see IWorkbench#GetElementFactory
 */
struct BERRY_UI_QT IElementFactory
{
  virtual ~IElementFactory();

  /**
   * Re-creates and returns an object from the state captured within the given
   * memento.
   * <p>
   * If the result is not null, it should be persistable; that is,
   * <pre>
   * result.getAdapter(org.eclipse.ui.IPersistableElement.class)
   * </pre>
   * should not return <code>null</code>. The caller takes ownership of the
   * result and must delete it when it is not needed any more.
   * </p>
   *
   * @param memento
   *            a memento containing the state for the object
   * @return an object, or <code>nullptr</code> if the element could not be
   *         created
   */
  virtual IAdaptable* CreateElement(const SmartPointer<IMemento>& memento) = 0;
};

}

Q_DECLARE_INTERFACE(berry::IElementFactory, "org.blueberry.ui.IElementFactory")

#endif // BERRYIELEMENTFACTORY_H
