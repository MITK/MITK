#include "mitkDisplayVectorInteractor.h"
#include "mitkOperation.h"
#include "mitkDisplayCoordinateOperation.h"
#include "mitkDisplayPositionEvent.h"
#include "mitkInteractionConst.h"
#include "mitkAction.h"

//##ModelId=3EF222410127
void mitk::DisplayVectorInteractor::ExecuteOperation(Operation* operation)
{
  DisplayCoordinateOperation* dcOperation = static_cast<DisplayCoordinateOperation*>(operation);
  if(dcOperation==NULL) return;
  
  switch(operation->GetOperationType())
  {
  case OpSELECTPOINT:
    m_Sender=dcOperation->GetRenderer();
    m_StartDisplayCoordinate=dcOperation->GetStartDisplayCoordinate();
    m_LastDisplayCoordinate=dcOperation->GetLastDisplayCoordinate();
    m_CurrentDisplayCoordinate=dcOperation->GetCurrentDisplayCoordinate();
    std::cout << m_CurrentDisplayCoordinate << std::endl;
    
    std::cout<<"Message from DisplayVectorInteractor.cpp::ExecuteOperation() : "
      << "StartDisplayCoordinate:" <<     m_StartDisplayCoordinate 
      << "LastDisplayCoordinate:" <<      m_LastDisplayCoordinate 
      << "CurrentDisplayCoordinate:" <<   m_CurrentDisplayCoordinate 
      << std::endl;
    
    break;
  }
  
}

//##ModelId=3EF2224401CB
bool mitk::DisplayVectorInteractor::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  bool ok=false;
  
  const DisplayPositionEvent* posEvent=dynamic_cast<const DisplayPositionEvent*>(stateEvent->GetEvent());
  if(posEvent==NULL) return false;

  const int actionId = action->GetActionId();
  switch(actionId%100)
  {
  case 0:
    {
      DisplayCoordinateOperation* doOp = new mitk::DisplayCoordinateOperation(OpTEST,  posEvent->GetSender(), posEvent->GetDisplayPosition(), posEvent->GetDisplayPosition(), posEvent->GetDisplayPosition());
      if (m_UndoEnabled)	//write to UndoMechanism
      {
        DisplayCoordinateOperation* undoOp = new DisplayCoordinateOperation(OpTEST, m_Sender, m_StartDisplayCoordinate, m_LastDisplayCoordinate, m_CurrentDisplayCoordinate);
        
        
        OperationEvent *operationEvent = new OperationEvent(this, doOp, undoOp,
          objectEventId, groupEventId);
        m_UndoController->SetOperationEvent(operationEvent);
      }
      
      //execute the Operation
      m_Destination->ExecuteOperation(doOp);
      ok = true;
      break;
    }
  case 1:
    {
      DisplayCoordinateOperation* doOp = new mitk::DisplayCoordinateOperation(OpSELECTPOINT,  posEvent->GetSender(), posEvent->GetDisplayPosition(), posEvent->GetDisplayPosition(), posEvent->GetDisplayPosition());
      if (m_UndoEnabled)	//write to UndoMechanism
      {
        DisplayCoordinateOperation* undoOp = new DisplayCoordinateOperation(OpSELECTPOINT, m_Sender, m_StartDisplayCoordinate, m_LastDisplayCoordinate, m_CurrentDisplayCoordinate);
        
        
        OperationEvent *operationEvent = new OperationEvent(this, doOp, undoOp,
          objectEventId, groupEventId);
        m_UndoController->SetOperationEvent(operationEvent);
      }
      
      //execute the Operation
      this->ExecuteOperation(doOp);
      ok = true;
      break;
    }
  case 2:
    {
      int opId;
      if(actionId<1200)
        opId=OpMOVE;
      else
        opId=OpZOOM;
      DisplayCoordinateOperation* doOp = new DisplayCoordinateOperation(opId,  m_Sender, m_StartDisplayCoordinate, m_CurrentDisplayCoordinate, posEvent->GetDisplayPosition());
      if (m_UndoEnabled)	//write to UndoMechanism
      {
        DisplayCoordinateOperation* undoOp = new mitk::DisplayCoordinateOperation(opId,  posEvent->GetSender(), m_StartDisplayCoordinate, m_LastDisplayCoordinate, m_CurrentDisplayCoordinate);
        
        
        OperationEvent *operationEvent = new OperationEvent(m_Destination, doOp, undoOp,
          objectEventId, groupEventId);
        m_UndoController->SetOperationEvent(operationEvent);
      }
      
      //make Operation
      m_LastDisplayCoordinate=m_CurrentDisplayCoordinate;
      m_CurrentDisplayCoordinate=posEvent->GetDisplayPosition();
      std::cout << m_CurrentDisplayCoordinate << std::endl;
      
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

//##ModelId=3EF2229F00F0
mitk::DisplayVectorInteractor::DisplayVectorInteractor(const char * type, mitk::OperationActor* destination)
: mitk::StateMachine(type), m_Destination(destination),
m_Sender(NULL),
m_StartDisplayCoordinate(0,0),
m_LastDisplayCoordinate(0,0),
m_CurrentDisplayCoordinate(0,0)
{
  if(m_Destination==NULL)
    m_Destination=this;
}


//##ModelId=3EF2229F010E
mitk::DisplayVectorInteractor::~DisplayVectorInteractor()
{
}


