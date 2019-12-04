/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYIHANDLERLISTENER_H_
#define BERRYIHANDLERLISTENER_H_

#include <berryMessage.h>

#include <org_blueberry_core_commands_Export.h>

namespace berry {

template<class T> class SmartPointer;

class HandlerEvent;

/**
 * An instance of this interface can be used by clients to receive notification
 * of changes to one or more instances of <code>IHandler</code>.
 * <p>
 * This interface may be implemented by clients.
 * </p>
 *
 * @see IHandler#addHandlerListener(IHandlerListener)
 * @see IHandler#removeHandlerListener(IHandlerListener)
 */
struct BERRY_COMMANDS IHandlerListener
{

  struct Events {

    typedef Message1<const SmartPointer<HandlerEvent>&> Event;

    Event handlerChanged;

    void AddListener(IHandlerListener* listener);
    void RemoveListener(IHandlerListener* listener);

  private:
    typedef MessageDelegate1<IHandlerListener, const SmartPointer<HandlerEvent>&> Delegate;
  };

  virtual ~IHandlerListener();

  /**
   * Notifies that one or more properties of an instance of
   * <code>IHandler</code> have changed. Specific details are described in
   * the <code>HandlerEvent</code>.
   *
   * @param handlerEvent
   *            the handler event. Guaranteed not to be <code>null</code>.
   */
  virtual void HandlerChanged(const SmartPointer<HandlerEvent>& handlerEvent) = 0;
};

}

#endif /* BERRYIHANDLERLISTENER_H_ */
