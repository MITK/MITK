/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIPERSISTABLEELEMENT_H
#define BERRYIPERSISTABLEELEMENT_H

#include <berryIPersistable.h>

namespace berry {

/**
 * Interface for asking an object to store its state in a memento.
 * <p>
 * This interface is typically included in interfaces where
 * persistance is required.
 * </p><p>
 * When the workbench is shutdown objects which implement this interface
 * will be persisted.  At this time the <code>GetFactoryId</code> method
 * is invoked to discover the id of the element factory that will be used
 * to re-create the object from a memento.  Then the <code>SaveState</code>
 * method is invoked to store the element data into a newly created memento.
 * The resulting mementos are collected up and written out to a single file.
 * </p>
 * <p>
 * During workbench startup these mementos are read from the file.  The
 * factory Id for each is retrieved and mapped to an <code>IElementFactory</code>
 * which has been registered in the element factory extension point.  If a
 * factory exists for the Id it will be engaged to re-create the original
 * object.
 * </p>
 *
 * @see IAdaptable
 * @see IMemento
 * @see IElementFactory
 */
struct IPersistableElement : public IPersistable
{
    /**
     * Returns the id of the element factory which should be used to re-create this
     * object.
     * <p>
     * Factory ids are declared in extensions to the standard extension point
     * <code>"org.eclipse.ui.elementFactories"</code>.
     * </p>
     *
     * @return the element factory id
     * @see IElementFactory
     */
  virtual QString GetFactoryId() const = 0;
};

}

#endif // BERRYIPERSISTABLEELEMENT_H
