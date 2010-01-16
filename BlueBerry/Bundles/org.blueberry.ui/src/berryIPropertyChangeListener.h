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

#ifndef BERRYIPROPERTYCHANGELISTENER_H_
#define BERRYIPROPERTYCHANGELISTENER_H_

#include <berryMacros.h>
#include <berryMessage.h>

#include "berryUiDll.h"
#include "berryPropertyChangeEvent.h"

namespace berry
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
struct BERRY_UI IPropertyChangeListener: public virtual Object
{

  berryInterfaceMacro(IPropertyChangeListener, berry);

  virtual ~IPropertyChangeListener()
  {
  }

  struct BERRY_UI Events {

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

  virtual void PropertyChange(Object::Pointer /*source*/, int /*propId*/) {}
};

template<typename R>
struct PropertyChangeAdapter: public IPropertyChangeListener
{

  typedef R Listener;
  typedef void
      (R::*Callback)(PropertyChangeEvent::Pointer);

  PropertyChangeAdapter(R* l, Callback c) :
    listener(l), callback(c)
  {
    poco_assert(listener);
    poco_assert(callback);
  }

  void PropertyChange(PropertyChangeEvent::Pointer event)
  {
    (listener->*callback)(event);
  }

private:

  Listener* listener;
  Callback callback;
};

template<typename R>
struct PropertyChangeIntAdapter: public IPropertyChangeListener
{

  typedef R Listener;
  typedef void
      (R::*Callback)(Object::Pointer, int);

  PropertyChangeIntAdapter(R* l, Callback c) :
    listener(l), callback(c)
  {
    poco_assert(listener);
    poco_assert(callback);
  }

  void PropertyChange(Object::Pointer source, int propId)
  {
    (listener->*callback)(source, propId);
  }

private:

  Listener* listener;
  Callback callback;
};

}

#endif /* BERRYIPROPERTYCHANGELISTENER_H_ */
