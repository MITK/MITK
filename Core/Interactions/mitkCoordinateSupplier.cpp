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


//##ModelId=3EDDD8F2001B
mitk::CoordinateSupplier::CoordinateSupplier(std::string type, mitk::OperationActor* operationActor)
: mitk::StateMachine(type), m_Destination(operationActor)
{
	m_Point.Fill(0.0);
}

//##ModelId=3EDDD8F20105
bool mitk::CoordinateSupplier::ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int groupEventId, int objectEventId)
{
    bool ok = false;
    if (m_Destination == NULL)
        return false;
	
    const PositionEvent* posEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
    if(posEvent==NULL) return false;

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
																	groupEventId, 
																	objectEventId);
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