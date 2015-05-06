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

#include <org_blueberry_ui_qt_Export.h>

#include <berryMacros.h>
#include <berryMessage.h>

#include "berrySelectionChangedEvent.h"

namespace berry
{

class SelectionChangedEvent;

/**
 * \ingroup org_blueberry_ui_qt
 *
 * A listener which is notified when a viewer's selection changes.
 *
 * @see ISelection
 * @see ISelectionProvider
 * @see SelectionChangedEvent
 */
struct BERRY_UI_QT ISelectionChangedListener
{

  struct BERRY_UI_QT Events {

    Message1<const SelectionChangedEvent::Pointer&> selectionChanged;

    void AddListener(ISelectionChangedListener* listener);
    void RemoveListener(ISelectionChangedListener* listener);

  private:

    typedef MessageDelegate1<ISelectionChangedListener, const SelectionChangedEvent::Pointer&> Delegate;
  };

  virtual ~ISelectionChangedListener();

    /**
     * Notifies that the selection has changed.
     *
     * @param event event object describing the change
     */
  virtual void SelectionChanged(const SelectionChangedEvent::Pointer& event) = 0;
};

}

#endif /*BERRYISELECTIONCHANGEDLISTENER_H_*/
