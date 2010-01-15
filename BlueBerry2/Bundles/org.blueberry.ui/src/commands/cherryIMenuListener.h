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


#ifndef CHERRYIMENULISTENER_H_
#define CHERRYIMENULISTENER_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>
#include <osgi/framework/Message.h>

#include "../cherryUiDll.h"

namespace cherry {

struct IMenuManager;

/**
 * A menu listener that gets informed when a menu is about to show.
 *
 * @see MenuManager#addMenuListener
 */
struct CHERRY_UI IMenuListener : public virtual Object {

  osgiInterfaceMacro(cherry::IMenuListener)

  struct Events {

    typedef Message1<SmartPointer<IMenuManager> > MenuEvent;

    MenuEvent menuAboutToShow;
    MenuEvent menuAboutToHide;

    void AddListener(IMenuListener::Pointer listener);
    void RemoveListener(IMenuListener::Pointer listener);

  private:
    typedef MessageDelegate1<IMenuListener, SmartPointer<IMenuManager> > Delegate;
  };

    /**
     * Notifies this listener that the menu is about to be shown by
     * the given menu manager.
     *
     * @param manager the menu manager
     */
   virtual void MenuAboutToShow(SmartPointer<IMenuManager> manager) = 0;

  /**
   * Notifies this listener that the menu is about to be hidden by
   * the given menu manager.
   *
   * @param manager the menu manager
   */
  virtual void MenuAboutToHide(SmartPointer<IMenuManager> manager) = 0;

};

}

#endif /* CHERRYIMENULISTENER_H_ */
