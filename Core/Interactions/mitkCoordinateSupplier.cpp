#include "mitkDisplayCoordinateOperation.h"
//has to be on top, otherwise compiler error!
#include "mitkCoordinateSupplier.h"
#include "mitkOperation.h"
#include "mitkOperationActor.h"
#include "mitkPointOperation.h"
#include "mitkPositionEvent.h"
//and not here!
#include <string>
#include "mitkInteractionConst.h"
#include "mitkAction.h"


//##ModelId=3F0189F0025B
mitk::CoordinateSupplier::CoordinateSupplier(const char * type, mitk::OperationActor* operationActor)
: mitk::StateMachine(type), m_Destination(operationActor)
{
}

//##ModelId=3F0189F00269
bool mitk::CoordinateSupplier::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent)
{
    bool ok = false;
    if (m_Destination == NULL)
        return false;
	
    const PositionEvent* posEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
    if(posEvent!=NULL)
    {
      switch (action->GetActionId())
      {
        case AcNEWPOINT:
        {
          m_OldPoint = posEvent->GetWorldPosition();

			    PointOperation* doOp = new mitk::PointOperation(OpADD, m_OldPoint, 0);
			    //Undo
          if (m_UndoEnabled)
          {
				    PointOperation* undoOp = new PointOperation(OpDELETE, m_OldPoint, 0);
            OperationEvent *operationEvent = new OperationEvent( m_Destination, doOp, undoOp );
            m_UndoController->SetOperationEvent(operationEvent);
          }
          //execute the Operation
			    m_Destination->ExecuteOperation(doOp);
          ok = true;
          break;
        }
        case AcINITMOVEMENT:
        {//move the point to the coordinate //not used, cause same to MovePoint... check xml-file
          mitk::Point3D movePoint = posEvent->GetWorldPosition();

          PointOperation* doOp = new mitk::PointOperation(OpMOVE, movePoint, 0);
          //execute the Operation
			    m_Destination->ExecuteOperation(doOp);
          ok = true;
          break;
        }
        case AcMOVEPOINT:
        {
          mitk::Point3D movePoint = posEvent->GetWorldPosition();

          PointOperation* doOp = new mitk::PointOperation(OpMOVE, movePoint, 0);
          //execute the Operation
			    m_Destination->ExecuteOperation(doOp);
          ok = true;
          break;
        }
        case AcFINISHMOVEMENT:
        {/*finishes a Movement from the coordinate supplier: 
          gets the lastpoint from the undolist and writes an undo-operation so 
          that the movement of the coordinatesupplier is undoable.*/
          mitk::Point3D movePoint = posEvent->GetWorldPosition();
          mitk::Point3D oldMovePoint; oldMovePoint.Fill(0);

          PointOperation* doOp = new mitk::PointOperation(OpMOVE, movePoint, 0);
          if (m_UndoEnabled )
          {
            //get the last Position from the UndoList
            OperationEvent *lastOperationEvent = m_UndoController->GetLastOfType(m_Destination, OpMOVE);
            if (lastOperationEvent != NULL)
            {
              PointOperation* lastOp = dynamic_cast<PointOperation *>(lastOperationEvent->GetOperation());
              if (lastOp != NULL)
              {
                oldMovePoint = lastOp->GetPoint();
              }
            }
            PointOperation* undoOp = new PointOperation(OpMOVE, oldMovePoint, 0);
            OperationEvent *operationEvent = new OperationEvent(m_Destination, doOp, undoOp);
            m_UndoController->SetOperationEvent(operationEvent);
          }
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
    
    const mitk::DisplayPositionEvent* displPosEvent = dynamic_cast<const mitk::DisplayPositionEvent *>(stateEvent->GetEvent());
    if(displPosEvent!=NULL)
    {
        return true;
    }

	return false;
}
