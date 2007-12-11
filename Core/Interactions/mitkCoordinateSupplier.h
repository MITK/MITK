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


#ifndef MITKCOORDINATESUPPLIER_H
#define MITKCOORDINATESUPPLIER_H

#include "mitkCommon.h"
#include "mitkStateMachine.h"
#include "mitkVector.h"

namespace mitk {

  class Operation;
  class OperationActor;

  //##ModelId=3F0189F00094
  //##Documentation
  //## @brief Interactor
  //##
  //## sends a Point, that can be processed in its own OperationActor
  //## @ingroup Interaction
  class CoordinateSupplier : public StateMachine
  {
  public:
    mitkClassMacro(CoordinateSupplier, StateMachine);
    mitkNewMacro2Param(Self, const char*, OperationActor*);

    itkGetConstReferenceMacro(CurrentPoint, Point3D);

  protected:
    //##Documentation
    //## @brief Constructor with needed arguments
    //## @param type: string, that describes the StateMachine-Scheme to take from all SM (see XML-File)
    //## @param operationActor: the Data, operations (+ points) are send to
    CoordinateSupplier(const char * type, OperationActor* operationActor);

    //##Documentation
    //## @brief executes the actions that are sent to this statemachine
    //## derived from StateMachine 
    virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent);

  private:
    //##ModelId=3F0189F0024A
    OperationActor* m_Destination;
    Point3D m_OldPoint;
    Point3D m_CurrentPoint;
  };

} // namespace mitk

#endif /* MITKCOORDINATESUPPLIER_H */
