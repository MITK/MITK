/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationInteractor_h
#define mitkSegmentationInteractor_h

#include "MitkSegmentationExports.h"
#include <mitkDisplayActionEventBroadcast.h>

namespace mitk
{
  /**
   *\class SegmentationInteractor
   *@brief Observer that adds interaction with a multi-label segmentation session to the default display interaction.
   *
   * At the moment, this includes changing the active label.
   *
   * @ingroup Interaction
   **/

  class MITKSEGMENTATION_EXPORT SegmentationInteractor : public DisplayActionEventBroadcast
  {
  public:

    mitkClassMacro(SegmentationInteractor, DisplayActionEventBroadcast);
    itkNewMacro(Self);

  protected:

    SegmentationInteractor();
    ~SegmentationInteractor() override;

    /**
     * Derived function.
     * Connects the action names used in the state machine pattern with functions implemented within
     * this InteractionEventObserver. This is only necessary here because the events are processed by the state machine.
     */
    void ConnectActionsAndFunctions() override;

  private:
    /**
    * @brief Reference to the service registration of the observer.
    *   This is needed to unregister the observer on unload.
    */
    us::ServiceRegistration<InteractionEventObserver> m_ServiceRegistration;

    void OnEnterRenderWindow(StateMachineAction*, InteractionEvent* interactionEvent);
    void OnLeaveRenderWindow(StateMachineAction*, InteractionEvent* interactionEvent);
  };
}
#endif
