#include "mitkLineInteractor.h"
#include "mitkStatusBar.h"
#include <mitkInteractionConst.h>
#include <mitkLineOperation.h>
#include <mitkPointOperation.h>
#include <mitkPositionEvent.h>
#include <mitkStateTransitionOperation.h>
#include <mitkDataTreeNode.h>

mitk::LineInteractor::LineInteractor(const char * type, DataTreeNode* dataTreeNode, int cellId, int pIdA, int pIdB)
: Interactor(type, dataTreeNode), m_CellId(cellId), m_PIdA(pIdA), m_PIdB(pIdB)
{
  m_PointA = new PointInteractor("pointinteractor", dataTreeNode, pIdA);
  m_PointB = new PointInteractor("pointinteractor", dataTreeNode, pIdB);
}

mitk::LineInteractor::~LineInteractor()
{
}

int mitk::LineInteractor::GetId()
{
  return m_Id;
}

int mitk::LineInteractor::GetCellId()
{
  return m_CellId;
}

int mitk::LineInteractor::GetPointIdA()
{
  return m_PIdA;
}

int mitk::LineInteractor::GetPointIdB()
{
  return m_PIdB;
}

//##Documentation
//## implemented SideEffects: SeDONOTHING, 
//## SeADD(adds a line between two points to the specified cell(m_CellId) in the data), 
//## SeINITMOVE, SeMOVE(moves the line and the two points), 
//## SeFINISHMOVE, SeREMOVE (removes the line, not the points),
//## SeREMOVEALL (removes the line and the two points), SeSELECT(select the line, not the points),
//## SeDESELECT(deselect the line, not the points)
bool mitk::LineInteractor::ExecuteSideEffect(int sideEffectId, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  bool ok = false;//for return type bool
  
  switch (sideEffectId)
	{
  case SeDONOTHING:
    ok = true;
  break;
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
  break;
  case SeMOVE:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
    if (posEvent == NULL)
      return false;

    mitk::ITKPoint3D newPoint;
    mitk::vm2itk(posEvent->GetWorldPosition(), newPoint);

    PointOperation* doOp = new mitk::PointOperation(OpMOVELINE, newPoint, m_Id);
    //execute the Operation
    //here no undo is stored, because the movement-steps aren't interesting. only the start and the end is interisting to store for undo.
    m_DataTreeNode->GetData()->ExecuteOperation(doOp);
    ok = true;
  }
  break;
  case SeFINISHMOVE:
  {
    mitk::PositionEvent const *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
    if (posEvent == NULL)
      return false;

    //finish the movement:
    // set undo-information and move it to the last position.
    mitk::ITKPoint3D newPoint;
    mitk::vm2itk(posEvent->GetWorldPosition(), newPoint);
    PointOperation* doOp = new mitk::PointOperation(OpMOVELINE, newPoint, m_Id);
    if ( m_UndoEnabled )
    {
      PointOperation* undoOp = new mitk::PointOperation(OpMOVELINE, m_LastPoint, m_Id);
      OperationEvent *operationEvent = new OperationEvent(m_DataTreeNode->GetData(),
                                                        doOp, undoOp,
                                                        objectEventId, groupEventId);
      m_UndoController->SetOperationEvent(operationEvent);
    }
    //execute the Operation
    m_DataTreeNode->GetData()->ExecuteOperation(doOp);

    //increase the GroupEventId, so that the raw-Undo goes to here
    //this->IncCurrGroupEventId();
   
    ok = true;
  }
  break;
  case SeSELECT:
  {
    mitk::ITKPoint3D newPoint;
    newPoint.Fill(0);
    LineOperation* doOp = new mitk::LineOperation(OpSELECTLINE, m_Id, m_PIdA, m_PIdB);
    if ( m_UndoEnabled )
    {
      LineOperation* undoOp = new mitk::LineOperation(OpDESELECTLINE, m_Id, m_PIdA, m_PIdB);
      OperationEvent *operationEvent = new OperationEvent(m_DataTreeNode->GetData(),
                                                        doOp, undoOp,
                                                        objectEventId, groupEventId);
      m_UndoController->SetOperationEvent(operationEvent);
    }
    //execute the Operation
    m_DataTreeNode->GetData()->ExecuteOperation(doOp);
    ok = true;
  }
  break;
  case SeDESELECT:
  {
    mitk::ITKPoint3D newPoint;
    newPoint.Fill(0);
    LineOperation* doOp = new mitk::LineOperation(OpDESELECTLINE, m_Id, m_PIdA, m_PIdB);
    if ( m_UndoEnabled )
    {
      LineOperation* undoOp = new mitk::LineOperation(OpSELECTLINE, m_Id, m_PIdA, m_PIdB);
      OperationEvent *operationEvent = new OperationEvent(m_DataTreeNode->GetData(),
                                                        doOp, undoOp,
                                                        objectEventId, groupEventId);
      m_UndoController->SetOperationEvent(operationEvent);
    }
    //execute the Operation
    m_DataTreeNode->GetData()->ExecuteOperation(doOp);
    ok = true;
  }
  break;

  default:
    (StatusBar::GetInstance())->DisplayText("Message from mitkLineInteractor: I do not understand the SideEffect!", 10000);
    return false;
    //a false here causes the statemachine to undo its last statechange.
    //otherwise it will end up in a different state, but without done SideEffect.
    //if a transition really has no SideEffect, than call donothing
  }
  return ok;
}
