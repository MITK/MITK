/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYABSTRACTHANDLER_H_
#define BERRYABSTRACTHANDLER_H_

#include "berryIHandler.h"
#include <org_blueberry_core_commands_Export.h>

namespace berry {

/**
 * <p>
 * This class is a partial implementation of <code>IHandler</code>. This
 * abstract implementation provides support for handler listeners. You should
 * subclass from this class unless you want to implement your own listener
 * support. Subclasses should call
 * {@link AbstractHandler#fireHandlerChanged(HandlerEvent)}when the handler
 * changes. Subclasses can also override {@link AbstractHandler#isEnabled()} and
 * {@link AbstractHandler#isHandled()}.
 * </p>
 *
 * @since 3.1
 */
class BERRY_COMMANDS AbstractHandler : public IHandler { // ,public EventManager {

public:
  berryObjectMacro(AbstractHandler);

private:

  /**
   * Track this base class enabled state.
   *
   * @since 3.4
   */
  bool baseEnabled;


public:

  AbstractHandler();

  /**
   * @see IHandler#addHandlerListener(IHandlerListener)
   */
//  void AddHandlerListener(final IHandlerListener handlerListener) {
//    addListenerObject(handlerListener);
//  }

  /**
   * The default implementation does nothing. Subclasses who attach listeners
   * to other objects are encouraged to detach them in this method.
   *
   * @see org.blueberry.core.commands.IHandler#dispose()
   */
  ~AbstractHandler() {
    // Do nothing.
  }

  /**
   * Whether this handler is capable of executing at this time. Subclasses may
   * override this method. If clients override this method they should also
   * consider overriding {@link #setEnabled(Object)} so they can be notified
   * about framework execution contexts.
   *
   * @return <code>true</code>
   * @see #setEnabled(Object)
   * @see #setBaseEnabled(boolean)
   */
  bool IsEnabled();

  /**
   * Whether this handler is capable of handling delegated responsibilities at
   * this time. Subclasses may override this method.
   *
   * @return <code>true</code>
   */
  bool IsHandled();

  /**
   * @see IHandler#removeHandlerListener(IHandlerListener)
   */
//  void RemoveHandlerListener(final IHandlerListener handlerListener) {
//    removeListenerObject(handlerListener);
//  }


protected:

  /**
   * Fires an event to all registered listeners describing changes to this
   * instance.
   * <p>
   * Subclasses may extend the definition of this method (i.e., if a different
   * type of listener can be attached to a subclass). This is used primarily
   * for support of <code>AbstractHandler</code> in
   * <code>org.blueberry.ui.workbench</code>, and clients should be wary of
   * overriding this behaviour. If this method is overridden, then the first
   * line of the method should be "<code>super.fireHandlerChanged(handlerEvent);</code>".
   * </p>
   *
   * @param handlerEvent
   *            the event describing changes to this instance. Must not be
   *            <code>null</code>.
   */
//  void FireHandlerChanged(final HandlerEvent handlerEvent) {
//    if (handlerEvent == null) {
//      throw new NullPointerException();
//    }
//
//    final Object[] listeners = getListeners();
//    for (int i = 0; i < listeners.length; i++) {
//      final IHandlerListener listener = (IHandlerListener) listeners[i];
//      listener.handlerChanged(handlerEvent);
//    }
//  }



  /**
   * Allow the default {@link #isEnabled()} to answer our enabled state. It
   * will fire a HandlerEvent if necessary. If clients use this method they
   * should also consider overriding {@link #setEnabled(Object)} so they can
   * be notified about framework execution contexts.
   *
   * @param state
   *            the enabled state
   * @since 3.4
   */
  void SetBaseEnabled(bool state);

  /**
   * <p>
   * Returns true iff there is one or more IHandlerListeners attached to this
   * AbstractHandler.
   * </p>
   * <p>
   * Subclasses may extend the definition of this method (i.e., if a different
   * type of listener can be attached to a subclass). This is used primarily
   * for support of <code>AbstractHandler</code> in
   * <code>org.blueberry.ui.workbench</code>, and clients should be wary of
   * overriding this behaviour. If this method is overridden, then the return
   * value should include "<code>super.hasListeners() ||</code>".
   * </p>
   *
   * @return true iff there is one or more IHandlerListeners attached to this
   *         AbstractHandler
   */
//  bool HasListeners() {
//    return isListenerAttached();
//  }


};

}

#endif /*BERRYABSTRACTHANDLER_H_*/
