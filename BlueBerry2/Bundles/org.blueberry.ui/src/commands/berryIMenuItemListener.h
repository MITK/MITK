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


#ifndef BERRYIMENUITEMLISTENER_H_
#define BERRYIMENUITEMLISTENER_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>
#include <osgi/framework/Message.h>

#include "../berryUiDll.h"

namespace berry {

struct IMenuItem;

/**
 * A menu item listener that gets informed when a menu item is about to
 * be disposed or selected.
 *
 * @see IMenuItem#AddMenuItemListener
 */
struct BERRY_UI IMenuItemListener : public virtual Object {

  osgiInterfaceMacro(berry::IMenuItemListener)

  struct Events {

    typedef Message1<const IMenuItem* > DisposedEvent;
    typedef Message1<SmartPointer<IMenuItem> > SelectedEvent;

    DisposedEvent menuItemDisposed;
    SelectedEvent menuItemSelected;

    void AddListener(IMenuItemListener::Pointer listener);
    void RemoveListener(IMenuItemListener::Pointer listener);

  private:
    typedef MessageDelegate1<IMenuItemListener, const IMenuItem* > DisposedDelegate;
    typedef MessageDelegate1<IMenuItemListener, SmartPointer<IMenuItem> > SelectedDelegate;
  };

    /**
     * Notifies this listener that the menu item is being disposed
     *
     * @param item the menu item
     */
   virtual void MenuItemDisposed(const IMenuItem* item) = 0;

  /**
   * Notifies this listener that the menu item is being selected
   *
   * @param item the menu item
   */
  virtual void MenuItemSelected(SmartPointer<IMenuItem> item) = 0;

};

}

#endif /* BERRYIMENUITEMLISTENER_H_ */
