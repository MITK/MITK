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

#ifndef BERRYISELECTIONCHANGEDLISTENER_H_
#define BERRYISELECTIONCHANGEDLISTENER_H_

#include <org_blueberry_ui_Export.h>

#include <berryMacros.h>
#include <berryMessage.h>

#include "berrySelectionChangedEvent.h"

namespace berry
{

class SelectionChangedEvent;

/**
 * \ingroup org_blueberry_ui
 *
 * A listener which is notified when a viewer's selection changes.
 *
 * @see ISelection
 * @see ISelectionProvider
 * @see SelectionChangedEvent
 */
struct BERRY_UI ISelectionChangedListener : public virtual Object {

  berryInterfaceMacro(ISelectionChangedListener, berry);

  struct BERRY_UI Events {

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

#endif /*BERRYISELECTIONCHANGEDLISTENER_H_*/
