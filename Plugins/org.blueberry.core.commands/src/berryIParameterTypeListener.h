/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYIPARAMETERTYPELISTENER_H_
#define BERRYIPARAMETERTYPELISTENER_H_

#include <berryMessage.h>

#include <org_blueberry_core_commands_Export.h>

namespace berry {

template<class T> class SmartPointer;

class ParameterTypeEvent;

/**
 * An instance of this interface can be used by clients to receive notification
 * of changes to one or more instances of {@link ParameterType}.
 * <p>
 * This interface may be implemented by clients.
 * </p>
 *
 * @see ParameterType#AddListener(IParameterTypeListener::Pointer)
 * @see ParameterType#RemoveListener(IParameterTypeListener::Pointer)
 */
struct BERRY_COMMANDS IParameterTypeListener
{

  struct Events {

    typedef Message1<const SmartPointer<const ParameterTypeEvent>& > Event;

    Event parameterTypeChanged;

    void AddListener(IParameterTypeListener* listener);
    void RemoveListener(IParameterTypeListener* listener);

    typedef MessageDelegate1<IParameterTypeListener, const SmartPointer<const ParameterTypeEvent>& > Delegate;
  };

  virtual ~IParameterTypeListener();

  /**
   * Notifies that one or more properties of an instance of
   * {@link ParameterType} have changed. Specific details are described in the
   * {@link ParameterTypeEvent}.
   *
   * @param parameterTypeEvent
   *            the event. Guaranteed not to be <code>null</code>.
   */
  virtual void ParameterTypeChanged(const SmartPointer<const ParameterTypeEvent>& parameterTypeEvent) = 0;

};


}

#endif /* BERRYIPARAMETERTYPELISTENER_H_ */
