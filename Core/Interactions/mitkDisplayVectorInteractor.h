/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

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

//##ModelId=3EF221E9001C
//##Documentation
//## @brief Interactor for displaying different slices in orthogonal views.
//## @ingroup Interaction
class DisplayVectorInteractor : public StateMachine
{
  public:
    mitkClassMacro(DisplayVectorInteractor, StateMachine);

    //##ModelId=3EF222410127
    virtual void ExecuteOperation(Operation* operation);
    //##ModelId=3EF2229F00F0
    DisplayVectorInteractor(const char * type, mitk::OperationActor* destination=NULL);

    //##ModelId=3EF2229F010E
    virtual ~DisplayVectorInteractor();

  protected:
    //##ModelId=3EF2224401CB
    virtual bool ExecuteAction(Action* action, mitk::StateEvent const* stateEvent);

  private:
    //##ModelId=3EF1D2A002DD
    BaseRenderer::Pointer m_Sender;

    //##ModelId=3EF1D2A00341
    mitk::Point2D m_StartDisplayCoordinate;
    //##ModelId=3F0177090324
    mitk::Point2D m_LastDisplayCoordinate;
    //##ModelId=3F0177090343
    mitk::Point2D m_CurrentDisplayCoordinate;

    //##ModelId=3EF231670106
    OperationActor* m_Destination;
};

} // namespace mitk



#endif /* MITKDISPLAYVECTORINTERACTOR_H_HEADER_INCLUDED_C10DC4EB */

