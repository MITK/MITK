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

#ifndef CHERRYIPROPERTYCHANGELISTENER_H_
#define CHERRYIPROPERTYCHANGELISTENER_H_

#include <cherryMacros.h>
#include <cherryMessage.h>

#include "cherryUiDll.h"
#include "cherryPropertyChangeEvent.h"

namespace cherry
{

/**
 * Listener for property changes.
 * <p>
 * Usage:
 * <pre>
 * IPropertyChangeListener listener =
 *   new IPropertyChangeListener() {
 *      public void propertyChange(PropertyChangeEvent event) {
 *         ... // code to deal with occurrence of property change
 *      }
 *   };
 * emitter.addPropertyChangeListener(listener);
 * ...
 * emitter.removePropertyChangeListener(listener);
 * </pre>
 * </p>
 */
struct CHERRY_UI IPropertyChangeListener: public virtual Object
{

  cherryInterfaceMacro(IPropertyChangeListener, cherry);

  virtual ~IPropertyChangeListener()
  {
  }

  struct Events {

    typedef Message1<PropertyChangeEvent::Pointer> EventType;

    EventType propertyChange;

    void AddListener(IPropertyChangeListener::Pointer listener);
    void RemoveListener(IPropertyChangeListener::Pointer listener);

  private:

    typedef MessageDelegate1<IPropertyChangeListener, PropertyChangeEvent::Pointer> Delegate;
  };

  /**
   * Notification that a property has changed.
   * <p>
   * This method gets called when the observed object fires a property
   * change event.
   * </p>
   *
   * @param event the property change event object describing which property
   * changed and how
   */
  virtual void PropertyChange(PropertyChangeEvent::Pointer event);

  virtual void PropertyChange(Object::Pointer source, int propId) {}
};

}

#endif /* CHERRYIPROPERTYCHANGELISTENER_H_ */
