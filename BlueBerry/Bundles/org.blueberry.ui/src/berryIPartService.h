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

#ifndef BERRYIPARTSERVICE_H_
#define BERRYIPARTSERVICE_H_

#include <org_blueberry_ui_Export.h>

#include "berryIWorkbenchPart.h"
#include "berryIWorkbenchPartReference.h"
#include "berryIPartListener.h"

namespace berry {

/**
 * \ingroup org_blueberry_ui
 *
 * A part service tracks the creation and activation of parts within a
 * workbench page.
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 *
 * @see IWorkbenchPage
 */
struct BERRY_UI IPartService {

  virtual ~IPartService();

  /**
   * Adds the given observer for part lifecycle events.
   * Has no effect if an identical listener is already registered.
   * <p>
   * <b>Note:</b> listeners should be removed when no longer necessary. If
   * not, they will be removed when the IServiceLocator used to acquire this
   * service is disposed.
   * </p>
   *
   * @param listener a part listener
   * @see #removePartListener(IPartListener)
   */
    virtual void AddPartListener(IPartListener::Pointer listener) = 0;

    /**
     * Returns the active part.
     *
     * @return the active part, or <code>null</code> if no part is currently active
     */
    virtual IWorkbenchPart::Pointer GetActivePart() = 0;

    /**
     * Returns the active part reference.
     *
     * @return the active part reference, or <code>null</code> if no part
     * is currently active
     */
    virtual IWorkbenchPartReference::Pointer GetActivePartReference() = 0;

    /**
     * Removes the given part listener.
     * Has no affect if an identical listener is not registered.
     *
     * @param listener a part listener
     */
    virtual void RemovePartListener(IPartListener::Pointer listener) = 0;

};

}  // namespace berry

#endif /*BERRYIPARTSERVICE_H_*/
