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


#ifndef MITKDISPLAYVECTORINTERACTOR_H_HEADER_INCLUDED_C10DC4EB
#define MITKDISPLAYVECTORINTERACTOR_H_HEADER_INCLUDED_C10DC4EB

#include "mitkCommon.h"
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

    OperationActor* m_Destination;
  };

} // namespace mitk



#endif /* MITKDISPLAYVECTORINTERACTOR_H_HEADER_INCLUDED_C10DC4EB */


