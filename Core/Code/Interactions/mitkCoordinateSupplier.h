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


#ifndef MITKCOORDINATESUPPLIER_H
#define MITKCOORDINATESUPPLIER_H

#include <MitkCoreExports.h>
#include "mitkStateMachine.h"
#include "mitkNumericTypes.h"

namespace mitk {

  class Operation;
  class OperationActor;

  //##Documentation
  //## @brief Interactor
  //##
  //## sends a Point, that can be processed in its own OperationActor
  //## @ingroup Interaction
  class MITK_CORE_EXPORT CoordinateSupplier : public StateMachine
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

    ~CoordinateSupplier();

    //##Documentation
    //## @brief executes the actions that are sent to this statemachine
    //## derived from StateMachine
    virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent);

  private:
    OperationActor* m_Destination;
    Point3D m_OldPoint;
    Point3D m_CurrentPoint;
  };

} // namespace mitk

#endif /* MITKCOORDINATESUPPLIER_H */
