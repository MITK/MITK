/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYISELECTIONPROVIDER_H_
#define BERRYISELECTIONPROVIDER_H_

#include <berryMacros.h>

#include "berryISelectionChangedListener.h"
#include "berryISelection.h"

namespace berry
{

/**
 * \ingroup org_blueberry_ui_qt
 *
 * Interface common to all objects that provide a selection.
 *
 * @see ISelection
 * @see ISelectionChangedListener
 * @see SelectionChangedEvent
 */
struct BERRY_UI_QT ISelectionProvider : public virtual Object
{

  berryObjectMacro(berry::ISelectionProvider);

  ~ISelectionProvider() override;

    /**
     * Adds a listener for selection changes in this selection provider.
     * Has no effect if an identical listener is already registered.
     *
     * @param listener a selection changed listener
     */
    virtual void AddSelectionChangedListener(ISelectionChangedListener* listener) = 0;

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
            ISelectionChangedListener* listener) = 0;

    /**
     * Sets the current selection for this selection provider.
     *
     * @param selection the new selection
     */
    virtual void SetSelection(const ISelection::ConstPointer& selection) = 0;
};

}

#endif /*BERRYISELECTIONPROVIDER_H_*/
