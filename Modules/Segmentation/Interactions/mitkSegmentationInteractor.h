/*===================================================================

 The Medical Imaging Interaction Toolkit (MITK)

 Copyright (c) German Cancer Research Center,
 Division of Medical and Biological Informatics.
 All rights reserved.

 This software is distributed WITHOUT ANY WARRANTY; without
 even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.

 See LICENSE.txt or http://www.mitk.org for details.

 ===================================================================*/

#ifndef mitkSegmentationInteractor_h
#define mitkSegmentationInteractor_h

#include "MitkSegmentationExports.h"
#include "mitkDisplayInteractor.h"

namespace mitk
{
  /**
   *\class SegmentationInteractor
   *@brief Observer that adds interaction with a segmentation session to the default display interaction.
   *
   * At the moment, this includes changing the active label.
   *
   * @ingroup Interaction
   **/

  class MITKSEGMENTATION_EXPORT SegmentationInteractor: public DisplayInteractor
  {
  public:
    mitkClassMacro(SegmentationInteractor, DisplayInteractor)
    itkNewMacro(Self)

  protected:
    SegmentationInteractor() {};
    virtual ~SegmentationInteractor() {};
    /**
     * Derived function.
     * Connects the action names used in the state machine pattern with functions implemented within
     * this InteractionEventObserver. This is only necessary here because the events are processed by the state machine.
     */
    virtual void ConnectActionsAndFunctions();

     /**
     * Changes the active label.
     */
    bool ChangeActiveLabel(StateMachineAction*, InteractionEvent*);
  };
}
#endif
