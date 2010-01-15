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


#ifndef CHERRYIMENUITEMLISTENER_H_
#define CHERRYIMENUITEMLISTENER_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>
#include <osgi/framework/Message.h>

#include "../cherryUiDll.h"

namespace cherry {

struct IMenuItem;

/**
 * A menu item listener that gets informed when a menu item is about to
 * be disposed or selected.
 *
 * @see IMenuItem#AddMenuItemListener
 */
struct CHERRY_UI IMenuItemListener : public virtual Object {

  osgiInterfaceMacro(cherry::IMenuItemListener)

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

#endif /* CHERRYIMENUITEMLISTENER_H_ */
