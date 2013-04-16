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


#ifndef BERRYISAVEABLESLIFECYCLELISTENER_H_
#define BERRYISAVEABLESLIFECYCLELISTENER_H_

#include "berrySaveablesLifecycleEvent.h"

#include <berryMessage.h>
#include <berryMacros.h>

namespace berry {

/**
 * Listener for events fired by implementers of {@link ISaveablesSource}.
 *
 * <p>
 * This service can be acquired from a part's service locator:
 *
 * <pre>
 * ISaveablesLifecycleListener listener = (ISaveablesLifecycleListener) getSite()
 *    .getService(ISaveablesLifecycleListener.class);
 * </pre>
 *
 * or, in the case of implementers of {@link ISaveablesSource} that are not a
 * part, from the workbench:
 *
 * <pre>
 * ISaveablesLifecycleListener listener = (ISaveablesLifecycleListener) workbench
 *    .getService(ISaveablesLifecycleListener.class);
 * </pre>
 *
 * <ul>
 * <li>This service is available globally.</li>
 * </ul>
 * </p>
 *
 * @since 3.2
 */
struct ISaveablesLifecycleListener : public virtual Object {

  berryInterfaceMacro(ISaveablesLifecycleListener, berry);

  struct Events {
    Message1<SaveablesLifecycleEvent::Pointer> lifecycleChange;

    void AddListener(ISaveablesLifecycleListener::Pointer listener);
    void RemoveListener(ISaveablesLifecycleListener::Pointer listener);

  private:

    typedef MessageDelegate1<ISaveablesLifecycleListener, SaveablesLifecycleEvent::Pointer> Delegate;
  };

  /**
   * Handle the given event. This method must be called on the UI thread.
   *
   * @param event
   */
  virtual void HandleLifecycleEvent(SaveablesLifecycleEvent::Pointer event) = 0;

};

}

#endif /* BERRYISAVEABLESLIFECYCLELISTENER_H_ */
