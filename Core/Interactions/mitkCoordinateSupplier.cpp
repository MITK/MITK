#include "mitkDisplayCoordinateOperation.h"
//has to be on top, otherwise compiler error!
#include "mitkCoordinateSupplier.h"
#include "Operation.h"
#include "OperationActor.h"
#include "mitkPointOperation.h"
#include "PositionEvent.h"
//and not here!
#include <string>
#include "mitkInteractionConst.h"


//##ModelId=3F0189F0025B
mitk::CoordinateSupplier::CoordinateSupplier(std::string type, mitk::OperationActor* operationActor)
: mitk::StateMachine(type), m_Destination(operationActor)
{
	m_Point.Fill(0.0);
}

//##ModelId=3F0189F00269
bool mitk::CoordinateSupplier::ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
    bool ok = false;
    if (m_Destination == NULL)
        return false;
	
    const PositionEvent* posEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
    if(posEvent!=NULL)
    {
        switch (sideEffectId)
        {
            case SeNEWPOINT:
            {
			    mitk::Point3D newPoint = posEvent->GetWorldPosition();
                vm2itk(newPoint, m_Point);

			    PointOperation* doOp = new mitk::PointOperation(OpADD, m_Point, 0);
			    //Undo
                if (m_UndoEnabled)
                {
				    PointOperation* undoOp = new PointOperation(OpDELETE, m_Point, 0);
                    OperationEvent *operationEvent = new OperationEvent(m_Destination, doOp, undoOp,
																	    objectEventId, groupEventId);
                    m_UndoController->SetOperationEvent(operationEvent);
                }
                //execute the Operation
			    m_Destination->ExecuteOperation(doOp);
                ok = true;
                break;
            }
            case SeMOVEPOINT:
            {
                mitk::ITKPoint3D movePoint;
                vm2itk(posEvent->GetWorldPosition(), movePoint);

                PointOperation* doOp = new mitk::PointOperation(OpMOVE, movePoint, 0);
			    //Undo
                if (m_UndoEnabled)
                {
				    PointOperation* undoOp = new PointOperation(OpMOVE, m_Point, 0);
                    OperationEvent *operationEvent = new OperationEvent(m_Destination, doOp, undoOp,
																	    objectEventId, groupEventId);
                    m_UndoController->SetOperationEvent(operationEvent);
                }
                m_Point = movePoint;//set the new point for storage
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
        std::cout<<"DisplayPositionEvent"<<std::endl;
        return true;
    }

	return false;
}