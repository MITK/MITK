#include "mitkVector.h"
#include "mitkDisplayVectorInteractor.h"
#include "Operation.h"
#include "mitkDisplayCoordinateOperation.h"
#include "PositionEvent.h"

//##ModelId=3EF222410127
void mitk::DisplayVectorInteractor::ExecuteOperation(Operation* operation)
{
    DisplayCoordinateOperation* dcOperation = dynamic_cast<DisplayCoordinateOperation*>(operation);
    if(dcOperation==NULL) return;

    switch(operation->GetExecutionId()%100)
    {
    case 1:
        m_Sender=dcOperation->GetRenderer();
        m_StartDisplayCoordinate=dcOperation->GetStartDisplayCoordinate();
        m_LastDisplayCoordinate=dcOperation->GetLastDisplayCoordinate();
        m_CurrentDisplayCoordinate=dcOperation->GetCurrentDisplayCoordinate();
            std::cout << m_CurrentDisplayCoordinate << std::endl;
        break;
    }

}

//##ModelId=3EF2224401CB
bool mitk::DisplayVectorInteractor::ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int groupEventId, int objectEventId)
{
    bool ok=false;

    const PositionEvent* posEvent=dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
    if(posEvent==NULL) return false;
    switch(sideEffectId%100)
    {
        case 1:
        {
            if (m_UndoEnabled)	//write to UndoMechanism
            {
                DisplayCoordinateOperation* undo = new DisplayCoordinateOperation(POINT, sideEffectId, m_Sender, m_StartDisplayCoordinate, m_LastDisplayCoordinate, m_CurrentDisplayCoordinate);
                DisplayCoordinateOperation* redo = new mitk::DisplayCoordinateOperation(POINT, sideEffectId,  posEvent->GetSender(), posEvent->GetDisplayPosition(), posEvent->GetDisplayPosition(), posEvent->GetDisplayPosition());

                OperationEvent *operationEvent = new OperationEvent(this, redo, undo,
																    groupEventId, 
																    objectEventId);
                //m_UndoController->SwitchUndoModel(LIMITEDLINEARUNDO);//no need to keep on switching and testing!
                m_UndoController->SetOperationEvent(operationEvent);
            }

            //make Operation
            mitk::DisplayCoordinateOperation* dcOperation = new mitk::DisplayCoordinateOperation(POINT, sideEffectId, posEvent->GetSender(), posEvent->GetDisplayPosition(), posEvent->GetDisplayPosition(), posEvent->GetDisplayPosition());
            //execute the Operation
            this->ExecuteOperation(dcOperation);
            ok = true;
            break;
        }
        case 2:
        {
            if (m_UndoEnabled)	//write to UndoMechanism
            {
                DisplayCoordinateOperation* undo = new mitk::DisplayCoordinateOperation(POINT, sideEffectId,  posEvent->GetSender(), m_StartDisplayCoordinate, m_LastDisplayCoordinate, m_CurrentDisplayCoordinate);
                DisplayCoordinateOperation* redo = new DisplayCoordinateOperation(POINT, sideEffectId, m_Sender, m_StartDisplayCoordinate, m_CurrentDisplayCoordinate, posEvent->GetDisplayPosition());

                OperationEvent *operationEvent = new OperationEvent(m_Destination, redo, undo,
																    groupEventId, 
																    objectEventId);
                //m_UndoController->SwitchUndoModel(LIMITEDLINEARUNDO);//no need to keep on switching and testing!
                m_UndoController->SetOperationEvent(operationEvent);
            }

            //make Operation
            m_LastDisplayCoordinate=m_CurrentDisplayCoordinate;
            m_CurrentDisplayCoordinate=posEvent->GetDisplayPosition();
            std::cout << m_CurrentDisplayCoordinate << std::endl;
            mitk::DisplayCoordinateOperation* dcOperation = new mitk::DisplayCoordinateOperation(POINT, sideEffectId, m_Sender, m_StartDisplayCoordinate, m_LastDisplayCoordinate, m_CurrentDisplayCoordinate);
            //execute the Operation
            m_Destination->ExecuteOperation(dcOperation);
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
mitk::DisplayVectorInteractor::DisplayVectorInteractor(std::string type, mitk::OperationActor* destination)
: mitk::Roi(type), m_Destination(destination),
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

