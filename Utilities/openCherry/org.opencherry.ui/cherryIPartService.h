/*=========================================================================
 
Program:   openCherry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef CHERRYIPARTSERVICE_H_
#define CHERRYIPARTSERVICE_H_

#include <org.opencherry.osgi/cherryDll.h>

namespace cherry {

/**
 * A part service tracks the creation and activation of parts within a
 * workbench page.
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 *
 * @see IWorkbenchPage
 */
struct CHERRY_API IPartService {

    /**
     * Adds the given listener for part lifecycle events.
     * Has no effect if an identical listener is already registered.
     *
     * @param listener a part listener
     */
    virtual void AddPartListener(IPartListener::Ptr listener) = 0;

    /**
     * Returns the active part.
     *
     * @return the active part, or <code>null</code> if no part is currently active
     */
    virtual IWorkbenchPart::Ptr GetActivePart() = 0;

    /**
     * Returns the active part reference.
     *
     * @return the active part reference, or <code>null</code> if no part
     * is currently active
     */
    virtual IWorkbenchPartReference::Ptr GetActivePartReference() = 0;

    /**
     * Removes the given part listener.
     * Has no affect if an identical listener is not registered.
     *
     * @param listener a part listener
     */
    virtual void RemovePartListener(IPartListener::Ptr listener) = 0;

};

}  // namespace cherry

#endif /*CHERRYIPARTSERVICE_H_*/
