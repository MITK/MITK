#include "mitkPointInteractor.h"
#include <mitkPointOperation.h>
#include <mitkPositionEvent.h>
#include <mitkOperationEvent.h>
#include "mitkStatusBar.h"
#include <mitkDataTreeNode.h>
#include <mitkPointSet.h>
#include <mitkInteractionConst.h>

mitk::PointInteractor::PointInteractor(const char * type, DataTreeNode* dataTreeNode, int id)
: Interactor(type, dataTreeNode), m_Id(id)
{
  m_Data = dynamic_cast<mitk::PointSet*>(dataTreeNode->GetData());
  if (m_Data == NULL)
  {
    (StatusBar::GetInstance())->DisplayText("Message from mitkPointInteractor: Wrong Data!", 10000);
    return;
    //@TODO: call exception!!!
  }
}

mitk::PointInteractor::~PointInteractor()
{}

int mitk::PointInteractor::GetId()
{
  return m_Id;
}

//##Documentation
//## implemented SideEffects: SeDONOTHING, 
//## SeADD (adds a points to the Data),
//## SeINITMOVE (stores start of movement for undo), 
//## SeMOVE (moves the point), 
//## SeFINISHMOVE (writes the movement to undo), 
//## SeREMOVE, SeSELECT, SeDESELECT
bool mitk::PointInteractor::ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
	bool ok = false;//for return type bool

  /*Each case must watch the type of the event!*/
  switch (sideEffectId)
	{
  case SeDONOTHING:
    ok = true;
	  break;
  case SeADD:
	/*
	declare a point in the data-structure
  */
	{
	  mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL) 
      return false;

		//converting from Point3D to itk::Point
		mitk::ITKPoint3D itkPoint;
		mitk::vm2itk(posEvent->GetWorldPosition(), itkPoint);

    //dexlare the operation and let the data execute it
    PointOperation* doOp = new mitk::PointOperation(OpINSERT, itkPoint, m_Id);
		if (m_UndoEnabled)
		{
		  //difference between OpDELETE and OpREMOVE is, that OpDELETE deletes a point at the end, and OpREMOVE deletes it from the given position
      //remove is better, cause we need the position to add or remove the point anyway. We can get the last position from size()
			PointOperation* undoOp = new mitk::PointOperation(OpREMOVE, itkPoint, m_Id);
			OperationEvent *operationEvent = new OperationEvent(m_Data,
																	doOp, undoOp,
																	objectEventId, groupEventId);
			m_UndoController->SetOperationEvent(operationEvent);
		}
		m_Data->ExecuteOperation(doOp);

		ok = true;
    break;
	}

  case SeINITMOVE:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL)
      return false;

    //start of the Movement is stored to calculate the undoKoordinate in FinishMovement
    mitk::vm2itk(posEvent->GetWorldPosition(), m_LastPoint);
    
    //initialize a value to calculate the movement through all MouseMoveEvents from MouseClick to MouseRelease
    ok = true;
    break;
  }
	
  case SeMOVE://moves the point
	{
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL)
      return false;

    mitk::ITKPoint3D newPoint;
		mitk::vm2itk(posEvent->GetWorldPosition(), newPoint);

    PointOperation* doOp = new mitk::PointOperation(OpMOVE, newPoint, m_Id);
    //execute the Operation
    //here no undo is stored, because the movement-steps aren't interesting. only the start and the end is interisting to store for undo.
		m_Data->ExecuteOperation(doOp);

    ok = true;
    break;
	}

  case SeFINISHMOVE:
	{
    mitk::PositionEvent const *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		if (posEvent == NULL)
      return false;

    //finish the movement:
    // set undo-information and move it to the last position.
    mitk::ITKPoint3D newPoint;
		mitk::vm2itk(posEvent->GetWorldPosition(), newPoint);

    PointOperation* doOp = new mitk::PointOperation(OpMOVE, newPoint, m_Id);
	  if ( m_UndoEnabled )
    {
      PointOperation* undoOp = new mitk::PointOperation(OpMOVE, m_LastPoint, m_Id);
			OperationEvent *operationEvent = new OperationEvent(m_Data,
												doOp, undoOp,
												objectEventId, groupEventId);
			m_UndoController->SetOperationEvent(operationEvent);
		}
		//execute the Operation
		m_Data->ExecuteOperation(doOp);

  //  //increase the GroupEventId, so that the raw-Undo goes to here
		//this->IncCurrGroupEventId();
		//ok = true;
	}
	break;

	case SeREMOVE://remove the given Point from the list
  {
    PointOperation* doOp = new mitk::PointOperation(OpREMOVE, ((PointSet*)(m_Data))->GetItkPoint(m_Id), m_Id);
		if (m_UndoEnabled)	//write to UndoMechanism
    {
			PointOperation* undoOp = new mitk::PointOperation(OpINSERT, ((PointSet*)(m_Data))->GetItkPoint(m_Id), m_Id);
		  OperationEvent *operationEvent = new OperationEvent(m_Data,
			  											doOp, undoOp,
															objectEventId, groupEventId);
			m_UndoController->SetOperationEvent(operationEvent);
		}
		//execute the Operation
		m_Data->ExecuteOperation(doOp);
   	ok = true;
  }
	break;

  case SeSELECT:
	{
    PointOperation* doOp = new mitk::PointOperation(OpSELECTPOINT, ((PointSet*)(m_Data))->GetItkPoint(m_Id), m_Id);
		if (m_UndoEnabled)	
    {
			PointOperation* undoOp = new mitk::PointOperation(OpDESELECTPOINT, ((PointSet*)(m_Data))->GetItkPoint(m_Id), m_Id);
			OperationEvent *operationEvent = new OperationEvent(m_Data,
																doOp, undoOp,
																objectEventId, groupEventId);
			m_UndoController->SetOperationEvent(operationEvent);
		}
		m_Data->ExecuteOperation(doOp);
		ok = true;
	}
	break;

  case SeDESELECT:
	{
    PointOperation* doOp = new mitk::PointOperation(OpDESELECTPOINT, ((PointSet*)(m_Data))->GetItkPoint(m_Id), m_Id);
		if (m_UndoEnabled)	
    {
			PointOperation* undoOp = new mitk::PointOperation(OpSELECTPOINT, ((PointSet*)(m_Data))->GetItkPoint(m_Id), m_Id);
			OperationEvent *operationEvent = new OperationEvent(m_Data,
																doOp, undoOp,
																objectEventId, groupEventId);
			m_UndoController->SetOperationEvent(operationEvent);
		}
		m_Data->ExecuteOperation(doOp);
		ok = true;
	}
	break;

	default:
    (StatusBar::GetInstance())->DisplayText("Message from mitkPointInteractor: I do not understand the SideEffect!", 10000);
    return false;
    //a false here causes the statemachine to undo its last statechange.
    //otherwise it will end up in a different state, but without done SideEffect.
    //if a transition really has no SideEffect, than call donothing
  }
  return ok;

}
