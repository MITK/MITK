/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYGUITKISELECTIONLISTENER_H_
#define BERRYGUITKISELECTIONLISTENER_H_

#include <berryMacros.h>
#include <berryMessage.h>

#include "berryGuiTkSelectionEvent.h"

namespace berry
{

namespace GuiTk
{

/**
 * Classes which implement this interface provide methods
 * that deal with the events that are generated when selection
 * occurs in a control.
 * <p>
 * After creating an instance of a class that implements
 * this interface it can be added to a control using the
 * <code>addSelectionListener</code> method and removed using
 * the <code>removeSelectionListener</code> method. When
 * selection occurs in a control the appropriate method
 * will be invoked.
 * </p>
 *
 * @see SelectionAdapter
 * @see SelectionEvent
 */
struct BERRY_UI_QT ISelectionListener: public virtual Object
{

  berryObjectMacro(berry::GuiTk::ISelectionListener);

  struct BERRY_UI_QT Events {

    typedef Message1<SelectionEvent::Pointer> EventType;

    EventType selected;
    EventType defaultSelected;

    void AddListener(ISelectionListener::Pointer listener);
    void RemoveListener(ISelectionListener::Pointer listener);

  private:
    typedef MessageDelegate1<ISelectionListener, SelectionEvent::Pointer> Delegate;
  };

  ~ISelectionListener() override;

  /**
   * Sent when selection occurs in the control.
   * <p>
   * For example, selection occurs in a List when the user selects
   * an item or items with the keyboard or mouse.  On some platforms,
   * the event occurs when a mouse button or key is pressed.  On others,
   * it happens when the mouse or key is released.  The exact key or
   * mouse gesture that causes this event is platform specific.
   * </p>
   *
   * @param e an event containing information about the selection
   */
  virtual void WidgetSelected(SelectionEvent::Pointer  /*e*/) {}

  /**
   * Sent when default selection occurs in the control.
   * <p>
   * For example, on some platforms default selection occurs in a List
   * when the user double-clicks an item or types return in a Text.
   * On some platforms, the event occurs when a mouse button or key is
   * pressed.  On others, it happens when the mouse or key is released.
   * The exact key or mouse gesture that causes this event is platform
   * specific.
   * </p>
   *
   * @param e an event containing information about the default selection
   */
  virtual void WidgetDefaultSelected(SelectionEvent::Pointer  /*e*/) {}
};

}

}

#endif /* BERRYGUITKISELECTIONLISTENER_H_ */
