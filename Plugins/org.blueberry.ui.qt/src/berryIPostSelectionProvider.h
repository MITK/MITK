/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIPOSTSELECTIONPROVIDER_H_
#define BERRYIPOSTSELECTIONPROVIDER_H_

#include "berryISelectionProvider.h"

namespace berry
{

/**
 * \ingroup org_blueberry_ui_qt
 *
 * Selection provider extension interface to allow providers
 * to notify about post selection changed events.
 * A post selection changed event is equivalent to selection changed event
 * if the selection change was triggered by the mouse, but it has a delay
 * if the selection change is triggered by keyboard navigation.
 *
 * @see ISelectionProvider
 */
struct BERRY_UI_QT IPostSelectionProvider : public ISelectionProvider
{

  berryObjectMacro(berry::IPostSelectionProvider);

  ~IPostSelectionProvider() override;

    /**
     * Adds a listener for post selection changes in this selection provider.
     * Has no effect if an identical listener is already registered.
     *
     * @param listener a selection changed listener
     */
  virtual void AddPostSelectionChangedListener(
            ISelectionChangedListener* listener) = 0;

    /**
     * Removes the given listener for post selection changes from this selection
     * provider.
     * Has no affect if an identical listener is not registered.
     *
     * @param listener a selection changed listener
     */
    virtual void RemovePostSelectionChangedListener(
            ISelectionChangedListener* listener) = 0;

};

}

#endif /*BERRYIPOSTSELECTIONPROVIDER_H_*/
