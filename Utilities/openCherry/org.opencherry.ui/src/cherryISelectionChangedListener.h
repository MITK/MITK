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

#ifndef CHERRYISELECTIONCHANGEDLISTENER_H_
#define CHERRYISELECTIONCHANGEDLISTENER_H_

#include "cherryUiDll.h"

#include <cherryMacros.h>
#include <cherryMessage.h>

#include "cherrySelectionChangedEvent.h"

namespace cherry
{

class SelectionChangedEvent;

/**
 * \ingroup org_opencherry_ui
 *
 * A listener which is notified when a viewer's selection changes.
 *
 * @see ISelection
 * @see ISelectionProvider
 * @see SelectionChangedEvent
 */
struct CHERRY_UI ISelectionChangedListener : public virtual Object {

  cherryInterfaceMacro(ISelectionChangedListener, cherry);

  struct Events {

    Message1<SelectionChangedEvent::Pointer > selectionChanged;

    void AddListener(ISelectionChangedListener::Pointer listener);
    void RemoveListener(ISelectionChangedListener::Pointer listener);

  private:

    typedef MessageDelegate1<ISelectionChangedListener, SelectionChangedEvent::Pointer> Delegate;
  };

    /**
     * Notifies that the selection has changed.
     *
     * @param event event object describing the change
     */
  virtual void SelectionChanged(SelectionChangedEvent::Pointer event) = 0;
};

}

#endif /*CHERRYISELECTIONCHANGEDLISTENER_H_*/
