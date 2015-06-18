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


#ifndef BERRYHANDLEREVENT_H_
#define BERRYHANDLEREVENT_H_

#include "common/berryAbstractBitSetEvent.h"
#include <org_blueberry_core_commands_Export.h>

namespace berry {

struct IHandler;

/**
 * An instance of this class describes changes to an instance of
 * <code>IHandler</code>.
 * <p>
 * This class is not intended to be extended by clients.
 * </p>
 *
 * @see IHandlerListener#HandlerChanged(HandlerEvent)
 */
class BERRY_COMMANDS HandlerEvent : public AbstractBitSetEvent {

public:

  berryObjectMacro(HandlerEvent);

  /**
   * Creates a new instance of this class.
   *
   * @param handler
   *            the instance of the interface that changed; must not be
     *            <code>null</code>.
   * @param enabledChanged
   *            Whether the enabled state of the handler has changed.
   * @param handledChanged
   *            Whether the handled state of the handler has changed.
   */
  HandlerEvent(const SmartPointer<IHandler> handler, bool enabledChanged,
      bool handledChanged);

  /**
   * Returns the instance of the interface that changed.
   *
   * @return the instance of the interface that changed. Guaranteed not to be
   *         <code>null</code>.
   */
  SmartPointer<IHandler> GetHandler() const;

  /**
   * Returns whether or not the enabled property changed.
   *
   * @return <code>true</code>, iff the enabled property changed.
   */
  bool IsEnabledChanged() const;

  /**
   * Returns whether or not the handled property changed.
   *
   * @return <code>true</code>, iff the handled property changed.
   */
  bool IsHandledChanged() const ;


private:

  /**
   * The bit used to represent whether the handler has changed its enabled
   * state.
   */
  static const int CHANGED_ENABLED; // = 1;

  /**
   * The bit used to represent whether the handler has changed its handled
   * state.
   */
  static const int CHANGED_HANDLED; // = 1 << 1;

  /**
   * The handler that changed; this value is never <code>null</code>.
   */
  const SmartPointer<IHandler> handler;


};

}

#endif /* BERRYHANDLEREVENT_H_ */
