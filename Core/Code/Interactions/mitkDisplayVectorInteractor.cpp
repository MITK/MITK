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


#include "mitkDisplayVectorInteractor.h"
#include "mitkOperation.h"
#include "mitkDisplayCoordinateOperation.h"
#include "mitkDisplayPositionEvent.h"
#include "mitkUndoController.h"
#include "mitkStateEvent.h"
#include "mitkInteractionConst.h"
#include "mitkAction.h"

void mitk::DisplayVectorInteractor::ExecuteOperation(Operation* itkNotUsed( operation ) )
{
  /*DisplayCoordinateOperation* dcOperation = static_cast<DisplayCoordinateOperation*>(operation);
  if(dcOperation==NULL) return;
  
  switch(operation->GetOperationType())
  {
  case OpSELECTPOINT:
    m_Sender=dcOperation->GetRenderer();
    m_StartDisplayCoordinate=dcOperation->GetStartDisplayCoordinate();
    m_LastDisplayCoordinate=dcOperation->GetLastDisplayCoordinate();
    m_CurrentDisplayCoordinate=dcOperation->GetCurrentDisplayCoordinate();
//    MITK_INFO << m_CurrentDisplayCoordinate << std::endl;
    
    MITK_INFO<<"Message from DisplayVectorInteractor.cpp::ExecuteOperation() : "
      << "StartDisplayCoordinate:" <<     m_StartDisplayCoordinate 
      << "LastDisplayCoordinate:" <<      m_LastDisplayCoordinate 
      << "CurrentDisplayCoordinate:" <<   m_CurrentDisplayCoordinate 
      << std::endl;
    
    break;
  }*/
  
}

bool mitk::DisplayVectorInteractor::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent)
{
  bool ok=false;
  
  const DisplayPositionEvent* posEvent=dynamic_cast<const DisplayPositionEvent*>(stateEvent->GetEvent());
  if(posEvent==NULL) return false;

  int actionId = action->GetActionId();
  //initzoom and initmove is the same!
  if (actionId == AcINITZOOM)
    actionId = AcINITMOVE;
  switch(actionId)
  {
  //case 0:
  //  {
  //    DisplayCoordinateOperation* doOp = new mitk::DisplayCoordinateOperation(OpTEST,  posEvent->GetSender(), posEvent->GetDisplayPosition(), posEvent->GetDisplayPosition(), posEvent->GetDisplayPosition());
  //    if (m_UndoEnabled)  //write to UndoMechanism
  //    {
  //      DisplayCoordinateOperation* undoOp = new DisplayCoordinateOperation(OpTEST, m_Sender, m_StartDisplayCoordinate, m_LastDisplayCoordinate, m_CurrentDisplayCoordinate);
  //      
  //      
  //      OperationEvent *operationEvent = new OperationEvent(this, doOp, undoOp);
  //      m_UndoController->SetOperationEvent(operationEvent);
  //    }
  //    
  //    //execute the Operation
  //    m_Destination->ExecuteOperation(doOp);
  //    ok = true;
  //    break;
  //  }
  case AcSENDCOORDINATES:
    {
      DisplayCoordinateOperation* doOp = new mitk::DisplayCoordinateOperation(OpSENDCOORDINATES,  posEvent->GetSender(), posEvent->GetDisplayPosition(), posEvent->GetDisplayPosition(), posEvent->GetDisplayPosition());
      m_Destination->ExecuteOperation(doOp);
      ok = true;
      break;
    }
  case AcINITMOVE:
    {
      m_Sender=posEvent->GetSender();

      mitk::Vector2D origin = m_Sender->GetDisplayGeometry()->GetOriginInMM();
       double scaleFactorMMPerDisplayUnit = m_Sender->GetDisplayGeometry()->GetScaleFactorMMPerDisplayUnit();

      m_StartDisplayCoordinate=posEvent->GetDisplayPosition();
      m_LastDisplayCoordinate=posEvent->GetDisplayPosition();
      m_CurrentDisplayCoordinate=posEvent->GetDisplayPosition();
      m_StartCoordinateInMM=mitk::Point2D( ( origin+m_StartDisplayCoordinate.GetVectorFromOrigin()*scaleFactorMMPerDisplayUnit ).GetDataPointer() );
      ok = true;
      break; if (m_UndoEnabled)  //write to UndoMechanism
      {
       // DisplayCoordinateOperation* doOp = new mitk::DisplayCoordinateOperation(OpMOVE,  m_Sender, m_StartDisplayCoordinate, m_StartDisplayCoordinate, posEvent->GetDisplayPosition());
        //DisplayCoordinateOperation* undoOp = new mitk::DisplayCoordinateOperation(OpMOVE,  posEvent->GetSender(), posEvent->GetDisplayPosition(), posEvent->GetDisplayPosition(), m_StartDisplayCoordinate);

        //OperationEvent *operationEvent = new OperationEvent(m_Destination, doOp, undoOp, "Move view");
       // m_UndoController->SetOperationEvent(operationEvent);
      }
    }
  case AcMOVE:
    {
      DisplayCoordinateOperation* doOp = new DisplayCoordinateOperation(OpMOVE,  m_Sender, m_StartDisplayCoordinate, m_CurrentDisplayCoordinate, posEvent->GetDisplayPosition());
      //make Operation
      m_LastDisplayCoordinate=m_CurrentDisplayCoordinate;
      m_CurrentDisplayCoordinate=posEvent->GetDisplayPosition();
      
      //execute the Operation
      m_Destination->ExecuteOperation(doOp);
      ok = true;
      break;
    }
  case AcFINISHMOVE:
    {     
      ok = true;
      break;
    }
  case AcZOOM:
    {
      DisplayCoordinateOperation* doOp = new DisplayCoordinateOperation(OpZOOM,  m_Sender, m_StartDisplayCoordinate, m_LastDisplayCoordinate, posEvent->GetDisplayPosition(),m_StartCoordinateInMM);
            
      //make Operation
      m_LastDisplayCoordinate=m_CurrentDisplayCoordinate;
      m_CurrentDisplayCoordinate=posEvent->GetDisplayPosition();
      //MITK_INFO << m_CurrentDisplayCoordinate << std::endl;
      
      //execute the Operation
      m_Destination->ExecuteOperation(doOp);
      ok = true;
      break;
    }
  default:
    ok = false;
    break;
  }
  return ok;
}

mitk::DisplayVectorInteractor::DisplayVectorInteractor(const char * type, mitk::OperationActor* destination)
  : mitk::StateMachine(type), m_Sender(NULL), m_Destination(destination)
{
  m_StartDisplayCoordinate.Fill(0);
  m_LastDisplayCoordinate.Fill(0);
  m_CurrentDisplayCoordinate.Fill(0);

  if(m_Destination==NULL)
    m_Destination=this;
}


mitk::DisplayVectorInteractor::~DisplayVectorInteractor()
{
}

