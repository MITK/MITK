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


#ifndef CHERRYIUIELEMENTLISTENER_H_
#define CHERRYIUIELEMENTLISTENER_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>
#include <osgi/framework/Message.h>

#include "../cherryUiDll.h"

namespace cherry {

using namespace osgi::framework;

struct UIElement;

/**
 * A user interface element listener that gets informed when an element is about to
 * be disposed or selected.
 *
 * @see UIElement#AddListener
 */
struct CHERRY_UI IUIElementListener : public virtual Object {

  osgiInterfaceMacro(cherry::IUIElementListener)

  struct Events {

    typedef Message1<UIElement* > DisposedEvent;
    typedef Message1<SmartPointer<UIElement> > SelectedEvent;

    DisposedEvent elementDisposed;
    SelectedEvent elementSelected;

    void AddListener(IUIElementListener::Pointer listener);
    void RemoveListener(IUIElementListener::Pointer listener);

  private:
    typedef MessageDelegate1<IUIElementListener, UIElement* > DisposedDelegate;
    typedef MessageDelegate1<IUIElementListener, SmartPointer<UIElement> > SelectedDelegate;
  };

    /**
     * Notifies this listener that the menu item is being disposed
     *
     * @param item the menu item
     */
   virtual void UIElementDisposed(UIElement* item) = 0;

  /**
   * Notifies this listener that the menu item is being selected
   *
   * @param item the menu item
   */
  virtual void UIElementSelected(SmartPointer<UIElement> item) = 0;

};

}


#endif /* CHERRYIUIELEMENTLISTENER_H_ */
