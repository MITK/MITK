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

#ifndef CHERRYGUITKISELECTIONLISTENER_H_
#define CHERRYGUITKISELECTIONLISTENER_H_

#include <cherryMacros.h>
#include <cherryMessage.h>

#include "cherryGuiTkSelectionEvent.h"

namespace cherry
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
struct CHERRY_UI ISelectionListener: public virtual Object
{

  cherryInterfaceMacro(ISelectionListener, cherry);

  struct CHERRY_UI Events {

    typedef Message1<SelectionEvent::Pointer> EventType;

    EventType selected;
    EventType defaultSelected;

    void AddListener(ISelectionListener::Pointer listener);
    void RemoveListener(ISelectionListener::Pointer listener);

  private:
    typedef MessageDelegate1<ISelectionListener, SelectionEvent::Pointer> Delegate;
  };

  virtual ~ISelectionListener() {}

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
  virtual void WidgetSelected(SelectionEvent::Pointer e) {}

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
  virtual void WidgetDefaultSelected(SelectionEvent::Pointer e) {}
};

}

}

#endif /* CHERRYGUITKISELECTIONLISTENER_H_ */
