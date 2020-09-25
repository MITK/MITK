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
#include "mitkDisplayInteractor.h"

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

  class MITKSEGMENTATION_EXPORT SegmentationInteractor : public DisplayInteractor
  {
  public:
    mitkClassMacro(SegmentationInteractor, DisplayInteractor);
    itkNewMacro(Self);

      protected : SegmentationInteractor(){};
    ~SegmentationInteractor() override{};
    /**
     * Derived function.
     * Connects the action names used in the state machine pattern with functions implemented within
     * this InteractionEventObserver. This is only necessary here because the events are processed by the state machine.
     */
    void ConnectActionsAndFunctions() override;

    /**
    * Changes the active label.
    */
    bool ChangeActiveLabel(StateMachineAction *, InteractionEvent *);
  };
}
#endif
