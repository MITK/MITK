#include "mitkPolygonInteractor.h"
#include <mitkInteractionConst.h>
#include <mitkPositionEvent.h>
#include <mitkLineOperation.h>
#include <mitkDataTreeNode.h>
#include <mitkMesh.h>
#include <mitkAction.h>
#include <mitkProperties.h>
#include <vtkTransform.h>


mitk::PolygonInteractor::PolygonInteractor(const char * type, DataTreeNode* dataTreeNode)
: HierarchicalInteractor(type, dataTreeNode)
{
  m_PrimStripInteractor = new mitk::PrimStripInteractor("primstripinteractor",dataTreeNode);
  this->AddInteractor((Interactor::Pointer)m_PrimStripInteractor);
}

mitk::PolygonInteractor::~PolygonInteractor()
{
  //delete m_PrimStripInteractor;
}

void mitk::PolygonInteractor::DeselectAllCells()  
{
  mitk::Mesh* mesh = dynamic_cast<mitk::Mesh*>(m_DataTreeNode->GetData());
	if (mesh == NULL)
		return;

  //deselect cells
  mitk::Mesh::DataType *itkMesh = mesh->GetMesh(); 
  mitk::Mesh::CellDataIterator cellDataIt, cellDataEnd;
  cellDataEnd = itkMesh->GetCellData()->End();
  for (cellDataIt = itkMesh->GetCellData()->Begin(); cellDataIt != cellDataEnd; cellDataIt++)
	{
    if ( cellDataIt->Value().selected )
		{
      mitk::LineOperation* doOp = new mitk::LineOperation(OpDESELECTCELL, cellDataIt->Index());
			if (m_UndoEnabled)
			{
				mitk::LineOperation* undoOp = new mitk::LineOperation(OpSELECTCELL, cellDataIt->Index());
				OperationEvent *operationEvent = new OperationEvent(mesh, doOp, undoOp);
				m_UndoController->SetOperationEvent(operationEvent);
			}
			mesh->ExecuteOperation(doOp);
		}
	}
}

float mitk::PolygonInteractor::CalculateJurisdiction(StateEvent const* stateEvent) const
{
  float returnValue = 0;

  float lowerJurisdiction = this->CalculateLowerJurisdiction(stateEvent);
  
  if (returnValue < lowerJurisdiction)
    returnValue = lowerJurisdiction;

  return returnValue;
}


bool mitk::PolygonInteractor::ExecuteAction( Action* action, mitk::StateEvent const* stateEvent )
{
  bool ok = false;//for return type bool
  
  //checking corresponding Data; has to be a PointSet or a subclass
  mitk::Mesh* mesh = dynamic_cast<mitk::Mesh*>(m_DataTreeNode->GetData());
	if (mesh == NULL)
		return false;

  switch (action->GetActionId())
	{
  case AcFINISHOBJECT:
    //finish the creation of the polygon
    {
      int cellId = mesh->SearchSelectedCell();
      LineOperation* lineDoOp = new mitk::LineOperation(OpCLOSECELL, cellId);
		  if (m_UndoEnabled)
		  {
		  	LineOperation* lineUndoOp = new mitk::LineOperation(OpOPENCELL, cellId);
			  OperationEvent *operationEvent = new OperationEvent(mesh, lineDoOp, lineUndoOp);
			  m_UndoController->SetOperationEvent(operationEvent);
		  }
		  //execute the Operation
      mesh->ExecuteOperation(lineDoOp );
    }
    ok = true;
    break;
  default:
    return Superclass::ExecuteAction(action, stateEvent);
  }
  return ok;
}
