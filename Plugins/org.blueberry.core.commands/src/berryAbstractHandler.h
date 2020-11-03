/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYABSTRACTHANDLER_H_
#define BERRYABSTRACTHANDLER_H_

#include "berryIHandler.h"
#include "berryIHandlerListener.h"

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
 */
class BERRY_COMMANDS AbstractHandler : public QObject, public IHandler { // ,public EventManager {

  Q_OBJECT
  Q_INTERFACES(berry::IHandler)

public:
  berryObjectMacro(AbstractHandler);

private:

  /**
   * Track this base class enabled state.
   */
  bool baseEnabled;

  IHandlerListener::Events handlerListeners;

public:

  AbstractHandler();

  /**
   * @see IHandler#addHandlerListener(IHandlerListener)
   */
  void AddHandlerListener(IHandlerListener* handlerListener) override;

  /**
   * The default implementation does nothing. Subclasses who attach listeners
   * to other objects are encouraged to detach them in this method.
   *
   * @see IHandler#Dispose()
   */
  void Dispose() override;

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
  bool IsEnabled() const override;

  /**
   * Called by the framework to allow the handler to update its enabled state
   * by extracting the same information available at execution time. Clients
   * may override if they need to extract information from the application
   * context.
   *
   * @param evaluationContext
   *            the application context. May be <code>null</code>
   * @see #SetBaseEnabled(bool)
   */
  void SetEnabled(const Object::Pointer& evaluationContext) override;

  /**
   * Whether this handler is capable of handling delegated responsibilities at
   * this time. Subclasses may override this method.
   *
   * @return <code>true</code>
   */
  bool IsHandled() const override;

  /**
   * @see IHandler#removeHandlerListener(IHandlerListener)
   */
  void RemoveHandlerListener(IHandlerListener* handlerListener) override;

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
  void FireHandlerChanged(const SmartPointer<HandlerEvent>& handlerEvent);

  /**
   * Allow the default {@link #isEnabled()} to answer our enabled state. It
   * will fire a HandlerEvent if necessary. If clients use this method they
   * should also consider overriding {@link #setEnabled(Object)} so they can
   * be notified about framework execution contexts.
   *
   * @param state
   *            the enabled state
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
   * <code>org.blueberry.ui.qt</code>, and clients should be wary of
   * overriding this behaviour. If this method is overridden, then the return
   * value should include "<code>super.hasListeners() ||</code>".
   * </p>
   *
   * @return true iff there is one or more IHandlerListeners attached to this
   *         AbstractHandler
   */
  virtual bool HasListeners() const;

};

}

#endif /*BERRYABSTRACTHANDLER_H_*/
