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


#ifndef MITKDISPLAYVECTORINTERACTOR_H_HEADER_INCLUDED_C10DC4EB
#define MITKDISPLAYVECTORINTERACTOR_H_HEADER_INCLUDED_C10DC4EB

#include <MitkExports.h>
#include "mitkBaseRenderer.h"
#include "mitkStateMachine.h"

namespace mitk {

  class Operation;
  class OperationActor;

  /**
  *@brief Interactor for displaying different slices in orthogonal views.
  * This includes the interaction of Zooming and Panning.
  * @ingroup Interaction
  **/
  class MITK_CORE_EXPORT DisplayVectorInteractor : public StateMachine
  {
  public:
    mitkClassMacro(DisplayVectorInteractor, StateMachine);
    mitkNewMacro2Param(Self, const char*, OperationActor*);

    /**
    * @brief Method derived from OperationActor to recieve and execute operations
    **/
    virtual void ExecuteOperation(Operation* operation);

    /**
     * @brief Method that returns how well this event can be handled by the DisplayVectorInteractor
     *        a right click into a 2D renderwindow can be handled very well!
     **/
    float CanHandleEvent(const StateEvent *stateEvent) const;

  protected:
    /**
    * @brief Default Constructor
    **/
    DisplayVectorInteractor(const char * type, mitk::OperationActor* destination=NULL);

    /**
    * @brief Default Destructor
    **/
    virtual ~DisplayVectorInteractor();

    /**
    * @brief Method derived from StateMachine to implement the own actions
    **/
    virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent);

  private:
    BaseRenderer::Pointer m_Sender;

    mitk::Point2D m_StartDisplayCoordinate;
    mitk::Point2D m_LastDisplayCoordinate;
    mitk::Point2D m_CurrentDisplayCoordinate;
    mitk::Point2D m_StartCoordinateInMM;

    OperationActor* m_Destination;
  };

} // namespace mitk



#endif /* MITKDISPLAYVECTORINTERACTOR_H_HEADER_INCLUDED_C10DC4EB */


