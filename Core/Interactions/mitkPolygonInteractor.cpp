#include "mitkPolygonInteractor.h"
#include "mitkStatusBar.h"
#include <mitkInteractionConst.h>
#include <mitkPositionEvent.h>
#include <mitkOperationEvent.h>
#include <mitkPointOperation.h>
#include <mitkDataTreeNode.h>

mitk::PolygonInteractor::PolygonInteractor(const char * type, DataTreeNode* dataTreeNode)
: Interactor(type, dataTreeNode), m_PointIdCount(0), m_LineIdCount(0)
{
}

mitk::PolygonInteractor::~PolygonInteractor()
{
}

int mitk::PolygonInteractor::GetId()
{
  return m_Id;
}


bool mitk::PolygonInteractor::ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  bool ok = false;//for return type bool
  
  switch (sideEffectId)
	{
  case SeDONOTHING:
    ok = true;
	  break;
  case SeINSERTPOINT:
    //add a new point to the List of Points
    {
      mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		  if (posEvent == NULL) 
        return false;

      //inserting the new interactor into the list
      mitk::PointInteractor::Pointer pointInteractor = new mitk::PointInteractor("pointinteractor", m_DataTreeNode, m_PointIdCount);
      m_PointList->insert(PointListType::value_type(m_PointIdCount, pointInteractor));

      //Now add the point to the data
		  mitk::ITKPoint3D itkPoint;
		  mitk::vm2itk(posEvent->GetWorldPosition(), itkPoint);
      mitk::PointOperation* doOp = new mitk::PointOperation(OpINSERT, itkPoint, m_PointIdCount);
		  if (m_UndoEnabled)
		  {
        mitk::PointOperation* undoOp = new mitk::PointOperation(OpREMOVE, itkPoint, m_PointIdCount);
			  OperationEvent *operationEvent = new OperationEvent(m_DataTreeNode->GetData(),
				  													doOp, undoOp,
					  												objectEventId, groupEventId);
			  m_UndoController->SetOperationEvent(operationEvent);
		  }
		  m_DataTreeNode->GetData()->ExecuteOperation(doOp);
	  
      ++m_PointIdCount;
    }
    ok = true;
	  break;

//  case SeREMOVE:
//  {
//    mitk::ITKPoint3D newPoint;
//    newPoint.Fill(0);
////critical, cause the inverse operation of removecell is not necesarily insertline. We don't know how the cell looked like when deleted.
//    PointOperation* doOp = new mitk::PointOperation(OpREMOVECELL, newPoint, m_Id);
//    if ( m_UndoEnabled )
//    {
//      LineOperation* undoOp = new mitk::LineOperation(OpINSERTLINE, m_PIdA, m_PIdB, m_Id);
//      OperationEvent *operationEvent = new OperationEvent(m_DataTreeNode->GetData(),
//                                                        doOp, undoOp,
//                                                        objectEventId, groupEventId);
//      m_UndoController->SetOperationEvent(operationEvent);
//    }
//    //execute the Operation
//    m_DataTreeNode->GetData()->ExecuteOperation(doOp);
//    ok = true;
//  }
//  break;
//  case SeREMOVEALL://remove Line and the two points
//  {
//    //removing the line
//    mitk::ITKPoint3D newPoint;
//    newPoint.Fill(0);
//    PointOperation* doOp = new mitk::PointOperation(OpREMOVECELL, newPoint, m_Id);
//    if ( m_UndoEnabled )
//    {
//      LineOperation* undoOp = new mitk::LineOperation(OpINSERTLINE, m_Id, m_PIdA, m_PIdB);
//      OperationEvent *operationEvent = new OperationEvent(m_DataTreeNode->GetData(),
//                                                        doOp, undoOp,
//                                                        objectEventId, groupEventId);
//      m_UndoController->SetOperationEvent(operationEvent);
//    }
//    //execute the Operation
//    m_DataTreeNode->GetData()->ExecuteOperation(doOp);
//
//    //now the two points. The StateMachines have to be deleted and the Data changed
//    //for undo to work, we have to store the state the machine was in when deleted
//    Operation* doOpA = new mitk::Operation(OpDELETE);
//    if ( m_UndoEnabled )
//    {
//      StateTransitionOperation* undoOpA = new mitk::StateTransitionOperation(OpUNDELETE, m_PointA->GetCurrentState());
//      OperationEvent *operationEventA = new OperationEvent(m_PointA,
//                                                        doOpA, undoOpA,
//                                                        objectEventId, groupEventId);
//      m_UndoController->SetOperationEvent(operationEventA);
//    }
//    //execute the Operation
//    m_PointA->ExecuteOperation(doOpA);
//    
//    Operation* doOpB = new mitk::Operation(OpDELETE);
//    if ( m_UndoEnabled )
//    {
//      StateTransitionOperation* undoOpB = new mitk::StateTransitionOperation(OpUNDELETE, m_PointB->GetCurrentState());
//      OperationEvent *operationEventB = new OperationEvent(m_PointB,
//                                                        doOpB, undoOpB,
//                                                        objectEventId, groupEventId);
//      m_UndoController->SetOperationEvent(operationEventB);
//    }
//    //execute the Operation
//    m_PointB->ExecuteOperation(doOp);
//
//    m_PointA = NULL;
//    m_PointB = NULL;
//
//
//    ok = true;
//  }
//  break;

  default:
    (StatusBar::GetInstance())->DisplayText("Message from mitkPolygonInteractor: I do not understand the SideEffect!", 10000);
    return false;
    //a false here causes the statemachine to undo its last statechange.
    //otherwise it will end up in a different state, but without done SideEffect.
    //if a transition really has no SideEffect, than call donothing
  }
  return ok;
}
