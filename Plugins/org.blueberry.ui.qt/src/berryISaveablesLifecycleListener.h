/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYISAVEABLESLIFECYCLELISTENER_H_
#define BERRYISAVEABLESLIFECYCLELISTENER_H_

#include "berrySaveablesLifecycleEvent.h"

#include <berryMessage.h>

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
 */
struct ISaveablesLifecycleListener : public virtual Object
{
  berryObjectMacro(berry::ISaveablesLifecycleListener);

  struct Events {
    Message1<const SaveablesLifecycleEvent::Pointer&> lifecycleChange;

    void AddListener(ISaveablesLifecycleListener* listener);
    void RemoveListener(ISaveablesLifecycleListener* listener);

  private:

    typedef MessageDelegate1<ISaveablesLifecycleListener, const SaveablesLifecycleEvent::Pointer&> Delegate;
  };

  ~ISaveablesLifecycleListener() override;

  /**
   * Handle the given event. This method must be called on the UI thread.
   *
   * @param event
   */
  virtual void HandleLifecycleEvent(const SaveablesLifecycleEvent::Pointer& event) = 0;

};

}

Q_DECLARE_INTERFACE(berry::ISaveablesLifecycleListener, "org.blueberry.ui.ISaveablesLifecycleListener")

#endif /* BERRYISAVEABLESLIFECYCLELISTENER_H_ */
