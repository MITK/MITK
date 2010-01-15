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


#ifndef CHERRYITOOLITEMLISTENER_H_
#define CHERRYITOOLITEMLISTENER_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>
#include <osgi/framework/Message.h>

#include "../cherryUiDll.h"

namespace cherry {

struct IToolItem;

/**
 * A tool item listener that gets informed when a tool item is about to
 * be disposed or selected.
 *
 * @see IMenuItem#AddMenuItemListener
 */
struct CHERRY_UI IToolItemListener : public virtual Object {

  osgiInterfaceMacro(cherry::IToolItemListener)

  struct Events {

    typedef Message1<const IToolItem* > DisposedEvent;
    typedef Message1<SmartPointer<IToolItem> > SelectedEvent;

    DisposedEvent toolItemDisposed;
    SelectedEvent toolItemSelected;

    void AddListener(IToolItemListener::Pointer listener);
    void RemoveListener(IToolItemListener::Pointer listener);

  private:
    typedef MessageDelegate1<IToolItemListener, const IToolItem* > DisposedDelegate;
    typedef MessageDelegate1<IToolItemListener, SmartPointer<IToolItem> > SelectedDelegate;
  };

    /**
     * Notifies this listener that the tool item is being disposed
     *
     * @param item the tool item
     */
   virtual void ToolItemDisposed(const IToolItem* item) = 0;

  /**
   * Notifies this listener that the tool item is being selected
   *
   * @param item the tool item
   */
  virtual void ToolItemSelected(SmartPointer<IToolItem> item) = 0;

};

}

#endif /* CHERRYITOOLITEMLISTENER_H_ */
