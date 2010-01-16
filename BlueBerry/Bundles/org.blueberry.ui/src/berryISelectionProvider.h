/*=========================================================================

 Program:   BlueBerry Platform
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

#ifndef BERRYISELECTIONPROVIDER_H_
#define BERRYISELECTIONPROVIDER_H_

#include <berryMacros.h>

#include "berryISelectionChangedListener.h"
#include "berryISelection.h"

namespace berry
{

/**
 * \ingroup org_blueberry_ui
 *
 * Interface common to all objects that provide a selection.
 *
 * @see ISelection
 * @see ISelectionChangedListener
 * @see SelectionChangedEvent
 */
struct BERRY_UI ISelectionProvider : public virtual Object {

  berryInterfaceMacro(ISelectionProvider, berry);

    /**
     * Adds a listener for selection changes in this selection provider.
     * Has no effect if an identical listener is already registered.
     *
     * @param listener a selection changed listener
     */
    virtual void AddSelectionChangedListener(ISelectionChangedListener::Pointer listener) = 0;

    /**
     * Returns the current selection for this provider.
     *
     * @return the current selection
     */
    virtual ISelection::ConstPointer GetSelection() const = 0;

    /**
     * Removes the given selection change listener from this selection provider.
     * Has no affect if an identical listener is not registered.
     *
     * @param listener a selection changed listener
     */
    virtual void RemoveSelectionChangedListener(
            ISelectionChangedListener::Pointer listener) = 0;

    /**
     * Sets the current selection for this selection provider.
     *
     * @param selection the new selection
     */
    virtual void SetSelection(ISelection::Pointer selection) = 0;
};

}

#endif /*BERRYISELECTIONPROVIDER_H_*/
