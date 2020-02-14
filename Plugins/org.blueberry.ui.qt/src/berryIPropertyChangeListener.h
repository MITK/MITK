/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIPROPERTYCHANGELISTENER_H_
#define BERRYIPROPERTYCHANGELISTENER_H_

#include <berryMacros.h>
#include <berryMessage.h>

#include <org_blueberry_ui_qt_Export.h>
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
struct BERRY_UI_QT IPropertyChangeListener
{

  virtual ~IPropertyChangeListener();

  struct BERRY_UI_QT Events {

    typedef Message1<const PropertyChangeEvent::Pointer&> EventType;

    EventType propertyChange;

    void AddListener(IPropertyChangeListener* listener);
    void RemoveListener(IPropertyChangeListener* listener);

  private:

    typedef MessageDelegate1<IPropertyChangeListener, const PropertyChangeEvent::Pointer&> Delegate;
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
  virtual void PropertyChange(const PropertyChangeEvent::Pointer& event);

  virtual void PropertyChange(const Object::Pointer& /*source*/, int /*propId*/) {}
};

template<typename R>
struct PropertyChangeAdapter : public IPropertyChangeListener
{

  typedef R Listener;
  typedef void
      (R::*Callback)(const PropertyChangeEvent::Pointer&);

  PropertyChangeAdapter(R* l, Callback c) :
    listener(l), callback(c)
  {
    poco_assert(listener);
    poco_assert(callback);
  }

  using IPropertyChangeListener::PropertyChange;

  void PropertyChange(const PropertyChangeEvent::Pointer& event) override
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
      (R::*Callback)(const Object::Pointer&, int);

  PropertyChangeIntAdapter(R* l, Callback c) :
    listener(l), callback(c)
  {
    poco_assert(listener);
    poco_assert(callback);
  }

  using IPropertyChangeListener::PropertyChange;

  void PropertyChange(const Object::Pointer& source, int propId) override
  {
    (listener->*callback)(source, propId);
  }

private:

  Listener* listener;
  Callback callback;
};

}

#endif /* BERRYIPROPERTYCHANGELISTENER_H_ */
