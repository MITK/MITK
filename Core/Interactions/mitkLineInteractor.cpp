#include "mitkLineInteractor.h"
#include "mitkStatusBar.h"
#include <mitkInteractionConst.h>

mitk::LineInteractor::LineInteractor(std::string type, DataTreeNode* dataTreeNode, int cellId, int pIdA, int pIdB)
: Interactor(type, dataTreeNode), m_CellId(cellId), m_PIdA(pIdA), m_PIdB(pIdB)
{
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
  case SeADD:
  {

  //  PointOperation* doOp = new mitk::PointOperation(OpINSERT, itkPoint, m_Id);
		//if (m_UndoEnabled)
		//{
		//  //difference between OpDELETE and OpREMOVE is, that OpDELETE deletes a point at the end, and OpREMOVE deletes it from the given position
  //    //remove is better, cause we need the position to add or remove the point anyway. We can get the last position from size()
		//	PointOperation* undoOp = new mitk::PointOperation(OpREMOVE, itkPoint, m_Id);
		//	OperationEvent *operationEvent = new OperationEvent(m_Data,
		//															doOp, undoOp,
		//															objectEventId, groupEventId);
		//	m_UndoController->SetOperationEvent(operationEvent);
		//}
		//pointSet->ExecuteOperation(doOp);
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
