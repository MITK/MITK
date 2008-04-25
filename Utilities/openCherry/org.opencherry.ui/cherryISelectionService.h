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

#ifndef CHERRYISELECTIONSERVICE_H_
#define CHERRYISELECTIONSERVICE_H_

#include "cherryUiDll.h"

namespace cherry {

/**
 * A selection service tracks the selection within an object.
 * <p>
 * A listener that wants to be notified when the selection becomes
 * <code>null</code> must implement the <code>INullSelectionListener</code>
 * interface.
 * </p>
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 * @see org.eclipse.ui.ISelectionListener
 * @see org.eclipse.ui.INullSelectionListener
 */
struct CHERRY_UI ISelectionService {
  
    /**
     * Adds the given selection listener.
     * Has no effect if an identical listener is already registered.
     *
     * @param listener a selection listener
     */
    virtual void AddSelectionListener(ISelectionListener::Ptr listener) = 0;

    /**
     * Adds a part-specific selection listener which is notified when selection changes
     * in the part with the given id. This is independent of part activation - the part
     * need not be active for notification to be sent.
     * <p>
     * When the part is created, the listener is passed the part's initial selection.
     * When the part is disposed, the listener is passed a <code>null</code> selection,
     * but only if the listener implements <code>INullSelectionListener</code>.
     * </p>
     * <p>
     * Note: This will not correctly track editor parts as each editor does 
     * not have a unique partId.
     * </p>
     *
     * @param partId the id of the part to track
     * @param listener a selection listener
     * @since 2.0
     */
    virtual void AddSelectionListener(const std::string& partId, ISelectionListener::Ptr listener) = 0;

    /**
     * Adds the given post selection listener.It is equivalent to selection 
     * changed if the selection was triggered by the mouse but it has a 
     * delay if the selection is triggered by the keyboard arrows.
     * Has no effect if an identical listener is already registered.
     * 
     * Note: Works only for StructuredViewer(s).
     *
     * @param listener a selection listener
     */
    virtual void AddPostSelectionListener(ISelectionListener::Ptr listener) = 0;

    /**
     * Adds a part-specific selection listener which is notified when selection changes
     * in the part with the given id. This is independent of part activation - the part
     * need not be active for notification to be sent.
     * <p>
     * When the part is created, the listener is passed the part's initial selection.
     * When the part is disposed, the listener is passed a <code>null</code> selection,
     * but only if the listener implements <code>INullSelectionListener</code>.
     * </p>
     * <p>
     * Note: This will not correctly track editor parts as each editor does 
     * not have a unique partId.
     * </p>
     *
     * @param partId the id of the part to track
     * @param listener a selection listener
     * @since 2.0
     */
    virtual void AddPostSelectionListener(const std::string& partId,
            ISelectionListener::Ptr listener);

    /**
     * Returns the current selection in the active part.  If the selection in the
     * active part is <em>undefined</em> (the active part has no selection provider)
     * the result will be <code>null</code>.
     *
     * @return the current selection, or <code>null</code> if undefined
     */
    virtual ISelection::Ptr GetSelection() = 0;

    /**
     * Returns the current selection in the part with the given id.  If the part is not open,
     * or if the selection in the active part is <em>undefined</em> (the active part has no selection provider)
     * the result will be <code>null</code>.
     *
     * @param partId the id of the part
     * @return the current selection, or <code>null</code> if undefined
     * @since 2.0
     */
    virtual ISelection::Ptr GetSelection(const std::string& partId) = 0;

    /**
     * Removes the given selection listener.
     * Has no effect if an identical listener is not registered.
     *
     * @param listener a selection listener
     */
    virtual void RemoveSelectionListener(ISelectionListener::Ptr listener) = 0;

    /**
     * Removes the given part-specific selection listener.
     * Has no effect if an identical listener is not registered for the given part id.
     *
     * @param partId the id of the part to track
     * @param listener a selection listener
     * @since 2.0
     */
    virtual void RemoveSelectionListener(const std::string& partId,
            ISelectionListener::Ptr listener) = 0;

    /**
     * Removes the given post selection listener.
     * Has no effect if an identical listener is not registered.
     *
     * @param listener a selection listener
     */
    virtual void RemovePostSelectionListener(ISelectionListener::Ptr listener) = 0;

    /**
     * Removes the given part-specific post selection listener.
     * Has no effect if an identical listener is not registered for the given part id.
     *
     * @param partId the id of the part to track
     * @param listener a selection listener
     * @since 2.0
     */
    virtual void RemovePostSelectionListener(const std::string& partId,
            ISelectionListener::Ptr listener) = 0;
};

}  // namespace cherry

#endif /*CHERRYISELECTIONSERVICE_H_*/
