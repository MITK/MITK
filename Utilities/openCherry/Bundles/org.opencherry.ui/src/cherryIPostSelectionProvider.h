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

#ifndef CHERRYIPOSTSELECTIONPROVIDER_H_
#define CHERRYIPOSTSELECTIONPROVIDER_H_

#include "cherryISelectionProvider.h"

namespace cherry
{

/**
 * \ingroup org_opencherry_ui
 *
 * Selection provider extension interface to allow providers
 * to notify about post selection changed events.
 * A post selection changed event is equivalent to selection changed event
 * if the selection change was triggered by the mouse, but it has a delay
 * if the selection change is triggered by keyboard navigation.
 *
 * @see ISelectionProvider
 *
 * @since 3.0
 */
struct CHERRY_UI IPostSelectionProvider : public ISelectionProvider {

  cherryInterfaceMacro(IPostSelectionProvider, cherry);

    /**
     * Adds a listener for post selection changes in this selection provider.
     * Has no effect if an identical listener is already registered.
     *
     * @param listener a selection changed listener
     */
  virtual void AddPostSelectionChangedListener(
            ISelectionChangedListener::Pointer listener) = 0;

    /**
     * Removes the given listener for post selection changes from this selection
     * provider.
     * Has no affect if an identical listener is not registered.
     *
     * @param listener a selection changed listener
     */
    virtual void RemovePostSelectionChangedListener(
            ISelectionChangedListener::Pointer listener) = 0;

};

}

#endif /*CHERRYIPOSTSELECTIONPROVIDER_H_*/
