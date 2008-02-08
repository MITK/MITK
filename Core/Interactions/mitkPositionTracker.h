/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef MITKPositionTracker_H
#define MITKPositionTracker_H

#include "mitkCommon.h"
#include "mitkStateMachine.h"


namespace mitk {

  class Operation;
  class OperationActor;
  
  //##Documentation
  //## @brief Interactor for Mouse Tracking
  //##
  //## Tracks a Point from an input device (usaly mouse pointer) 
  //## @ingroup Interaction
  class MITK_CORE_EXPORT PositionTracker : public StateMachine
  {
  public:
    mitkClassMacro(PositionTracker, StateMachine);
    mitkNewMacro2Param(Self, const char*, OperationActor*);

  protected:
    //##Documentation
    //## @brief Constructor with needed arguments
    //## @param type: string, that describes the StateMachine-Scheme to take from all SM (see XML-File)
    //## @param operationActor: the Data, operations (+ points) are send to
    PositionTracker(const char * type, OperationActor* operationActor);

    //##Documentation
    //## @brief executes the actions that are sent to this statemachine
    //## derived from StateMachine 
    virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent);

  };

} // namespace mitk

#endif /* MITKPositionTracker_H */
