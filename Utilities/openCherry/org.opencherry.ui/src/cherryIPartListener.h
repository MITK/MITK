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

#ifndef CHERRYIPARTLISTENER_H_
#define CHERRYIPARTLISTENER_H_

#include <org.opencherry.osgi/cherryMacros.h>

#include "cherryUiDll.h"
#include "cherryIWorkbenchPartReference.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 * 
 * Interface for listening to part lifecycle events.
 * <p> 
 * This interface may be implemented by clients.
 * </p>
 *
 * @see IPartService#AddPartListener(IPartListener)
 */
struct CHERRY_UI IPartListener : public virtual Object {
  
  cherryClassMacro(IPartListener);

    /**
     * Notifies this listener that the given part has been activated.
     *
     * @param partRef the part that was activated
     * @see IWorkbenchPage#activate
     */
    virtual void PartActivated(IWorkbenchPartReference::Pointer partRef) = 0;

    /**
     * Notifies this listener that the given part has been brought to the top.
     * <p>
     * These events occur when an editor is brought to the top in the editor area,
     * or when a view is brought to the top in a page book with multiple views.
     * They are normally only sent when a part is brought to the top 
     * programmatically (via <code>IPerspective.bringToTop</code>). When a part is
     * activated by the user clicking on it, only <code>partActivated</code> is sent.
     * </p>
     *
     * @param partRef the part that was surfaced
     * @see IWorkbenchPage#bringToTop
     */
    virtual void PartBroughtToTop(IWorkbenchPartReference::Pointer partRef) = 0;

    /**
     * Notifies this listener that the given part has been closed.
     * <p>
     * Note that if other perspectives in the same page share the view,
     * this notification is not sent.  It is only sent when the view
     * is being removed from the page entirely (it is being disposed).
     * </p>
     *
     * @param partRef the part that was closed
     * @see IWorkbenchPage#hideView
     */
    virtual void PartClosed(IWorkbenchPartReference::Pointer partRef) = 0;

    /**
     * Notifies this listener that the given part has been deactivated.
     *
     * @param partRef the part that was deactivated
     * @see IWorkbenchPage#activate
     */
    virtual void PartDeactivated(IWorkbenchPartReference::Pointer partRef) = 0;

    /**
     * Notifies this listener that the given part has been opened.
     * <p>
     * Note that if other perspectives in the same page share the view,
     * this notification is not sent.  It is only sent when the view
     * is being newly opened in the page (it is being created).
     * </p>
     *
     * @param partRef the part that was opened
     * @see IWorkbenchPage#showView
     */
    virtual void PartOpened(IWorkbenchPartReference::Pointer partRef) = 0;

    /**
     * Notifies this listener that the given part is hidden or obscured by another part.
     *
     * @param partRef the part that is hidden or obscured by another part
     */
    virtual void PartHidden(IWorkbenchPartReference::Pointer partRef) = 0;

    /**
     * Notifies this listener that the given part is visible.
     *
     * @param partRef the part that is visible
     */
    virtual void PartVisible(IWorkbenchPartReference::Pointer partRef) = 0;

    /**
     * Notifies this listener that the given part's input was changed.
     *
     * @param partRef the part whose input was changed
     */
    virtual void PartInputChanged(IWorkbenchPartReference::Pointer partRef) = 0;
};

}  // namespace cherry

#endif /*CHERRYIPARTLISTENER_H_*/
