#include "mitkPolygonInteractor.h"
#include "mitkStatusBar.h"
#include <mitkInteractionConst.h>
#include <mitkPositionEvent.h>
#include <mitkOperationEvent.h>
#include <mitkStateEvent.h>
#include <mitkPointOperation.h>
#include <mitkLineOperation.h>
#include <mitkDataTreeNode.h>
#include <mitkMesh.h>
#include <vtkPolyData.h>
#include <mitkAction.h>
#include <mitkEventMapper.h>
#include <mitkProperties.h>


mitk::PolygonInteractor::PolygonInteractor(const char * type, DataTreeNode* dataTreeNode)
: HierarchicalInteractor(type, dataTreeNode)
{
  m_LineInteractor = new mitk::LineInteractor("lineinteractor",dataTreeNode);
  this->AddInteractor((Interactor::Pointer)m_LineInteractor);
}

mitk::PolygonInteractor::~PolygonInteractor()
{
  //delete m_LineInteractor;
}

void mitk::PolygonInteractor::DeselectAllCells(int objectEventId, int groupEventId)  
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
				OperationEvent *operationEvent = new OperationEvent(mesh,
																	doOp, undoOp,
																	objectEventId, groupEventId);
				m_UndoController->SetOperationEvent(operationEvent);
			}
			mesh->ExecuteOperation(doOp);
		}
	}
}

bool mitk::PolygonInteractor::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  bool ok = false;//for return type bool
  
  //checking corresponding Data; has to be a PointSet or a subclass
  mitk::Mesh* mesh = dynamic_cast<mitk::Mesh*>(m_DataTreeNode->GetData());
	if (mesh == NULL)
		return false;

  switch (action->GetActionId())
	{
  case AcDONOTHING:
    ok = true;
	  break;
  //case AcTRANSMITEVENT:
  //  {
  //    //due to the use of guards-states the eventId can be changed from original to internal EventIds e.g. StYES.
  //    //so we have remap the event and transmitt the original event with proper id
  //    ok = m_LineInteractor->HandleEvent(mitk::EventMapper::RefreshStateEvent(const_cast<StateEvent*>(stateEvent)), objectEventId, groupEventId );
  //    //check the state of the machine and according to that change the state/mode of this statemachine
  //    int mode = m_LineInteractor->GetMode();
  //    if (mode == mitk::Interactor::SMSELECTED ||
  //      mode == mitk::Interactor::SMSUBSELECTED)
  //      this->SetMode(mitk::Interactor::SMSUBSELECTED);
  //    else 
  //      this->SetMode(mitk::Interactor::SMDESELECTED);
  //  }
	 // break;
  case AcINITNEWOBJECT:
    {
      //get the next cellId and set m_CurrentCellId
      m_CurrentCellId = mesh->GetNewCellId();

      //now reserv a new cell in m_ItkData
      LineOperation* doOp = new mitk::LineOperation(OpNEWCELL, m_CurrentCellId);
		  if (m_UndoEnabled)
		  {
			  LineOperation* undoOp = new mitk::LineOperation(OpDELETECELL, m_CurrentCellId);
			  OperationEvent *operationEvent = new OperationEvent(mesh,
																	  doOp, undoOp,
																	  objectEventId, groupEventId);
			  m_UndoController->SetOperationEvent(operationEvent);
		  }
      mesh->ExecuteOperation(doOp);
    }
    ok = true;
    break;
  case AcFINISHOBJECT:
    //finish the creation of the polygon
    {
      LineOperation* lineDoOp = new mitk::LineOperation(OpCLOSECELL, m_CurrentCellId);
		  if (m_UndoEnabled)
		  {
		  	LineOperation* lineUndoOp = new mitk::LineOperation(OpOPENCELL, m_CurrentCellId);
			  OperationEvent *operationEvent = new OperationEvent(mesh,
				  													lineDoOp, lineUndoOp,
					  												objectEventId, groupEventId);
			  m_UndoController->SetOperationEvent(operationEvent);
		  }
		  //execute the Operation
      mesh->ExecuteOperation(lineDoOp );
    }
    ok = true;
    break;
  case AcCHECKLINE:
    //check, if a line is hit
    {
      mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
			if (posEvent == NULL)
        return false;

      //converting from Point3D to itk::Point
      mitk::Point3D worldPoint = posEvent->GetWorldPosition();

      //searching for a point
      unsigned long lineId = -1;
      unsigned long cellId = -1;
      int PRECISION = 1;
      mitk::IntProperty *precision = dynamic_cast<IntProperty*>(action->GetProperty("PRECISION"));
      if (precision != NULL)
      {
        PRECISION = precision->GetValue();
      }

      if(mesh->SearchLine(worldPoint, PRECISION, lineId, cellId))//line found
      {
        m_CurrentCellId = cellId;
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(StYES, stateEvent->GetEvent());
        this->HandleEvent(newStateEvent, objectEventId, groupEventId );
        ok = true;
      }
      else
		  {
			  //new Event with information NO
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(StNO, stateEvent->GetEvent());
        this->HandleEvent(newStateEvent, objectEventId, groupEventId );
			  ok = true;
			}
    }
    break;
  case AcCHECKBOUNDINGBOX:
    //check if the MousePosition lies inside the BoundingBox of the current Cell
    {
      mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
  		if (posEvent == NULL) 
        return false;

      mitk::Point3D worldPoint = posEvent->GetWorldPosition();
      
      //Axis-aligned bounding box(AABB) 
      mitk::Mesh::DataType::BoundingBoxPointer aABB = mesh->GetBoundingBoxFromCell(m_CurrentCellId);
      if (aABB.IsNull())
        return false;

      //check if the given point lies inside the boundingbox
      if (aABB->IsInside(worldPoint))
      {
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
        this->HandleEvent( newStateEvent, objectEventId, groupEventId );

      }
      else
      {
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
        this->HandleEvent(newStateEvent, objectEventId, groupEventId );
      }
    }
    ok = true;
    break;
  case AcCHECKOBJECT:
    {
      //picking if this object is hit. then set the m_CurrentCellId Variable
      //this object is hit, if the transmitted position of the event is hitting a point or a line of that cell
      mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
  		if (posEvent == NULL) 
        return false;

      bool hit = true;
      //var's for EvaluatePosition
      mitk::Point3D point = posEvent->GetWorldPosition();
      unsigned long cellId = -1;
      
      //check wheather the mesh is hit.
      if (mesh->EvaluatePosition(point, cellId))
      {
        m_CurrentCellId = cellId;
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
        this->HandleEvent( newStateEvent, objectEventId, groupEventId );

      }
      else
      {
        m_CurrentCellId = 0;
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
        this->HandleEvent(newStateEvent, objectEventId, groupEventId );
      }
    }
    ok = true;
    break;
  case AcSELECTCELL:
    {
      LineOperation* lineDoOp = new mitk::LineOperation(OpSELECTCELL, m_CurrentCellId);
		  if (m_UndoEnabled)
		  {
		  	LineOperation* lineUndoOp = new mitk::LineOperation(OpDESELECTCELL, m_CurrentCellId);
			  OperationEvent *operationEvent = new OperationEvent(mesh,
				  													lineDoOp, lineUndoOp,
					  												objectEventId, groupEventId);
			  m_UndoController->SetOperationEvent(operationEvent);
		  }
      mesh->ExecuteOperation(lineDoOp );
    }
    ok = true;
    break;
  case AcDESELECTCELL:
    {
      //if number of cell is >0
      if (mesh->GetNumberOfCells()>0)
      {
        LineOperation* lineDoOp = new mitk::LineOperation(OpDESELECTCELL, m_CurrentCellId);
		    if (m_UndoEnabled)
		    {
		  	  LineOperation* lineUndoOp = new mitk::LineOperation(OpSELECTCELL, m_CurrentCellId);
			    OperationEvent *operationEvent = new OperationEvent(mesh,
				  													  lineDoOp, lineUndoOp,
					  												  objectEventId, groupEventId);
			    m_UndoController->SetOperationEvent(operationEvent);
		    }
        mesh->ExecuteOperation(lineDoOp );
      }
    }
    ok = true;
    break;
  case AcINITEDITOBJECT:
    ok = true;
    break;
  case AcINITMOVEMENT:
    //prepare everything for movement of one cell
    {
      mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		  if (posEvent == NULL)
        return false;
    
      //start of the Movement is stored to calculate the undoCoordinate in FinishMovement
      m_MovementStart = posEvent->GetWorldPosition();
      m_OldPoint = m_MovementStart;
    }
    ok = true;
    break;
  case AcMOVE:
    //move the cell without undo
    {
      mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
	  	if (posEvent == NULL)
        return false;

      mitk::Point3D newPoint = posEvent->GetWorldPosition();

      //calculate a vector between the oldPoint and the newPoint
      Vector3D vector = newPoint - m_OldPoint;
      
      CellOperation* doOp = new mitk::CellOperation(OpMOVECELL, m_CurrentCellId, vector);

      m_OldPoint = newPoint;
      
      //execute the Operation
      //here no undo is stored. only the start and the end is stored for undo.
		  mesh->ExecuteOperation(doOp);
    }
    ok = true;
    break;
  case AcFINISHMOVEMENT:
    //move the cell to the final position and send undo-information
    {
      mitk::PositionEvent const *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		  if (posEvent == NULL)
        return false;

      //finish the movement:
      // set undo-information and move it to the last position.
      mitk::Point3D newPoint = posEvent->GetWorldPosition();

      Vector3D vector = newPoint - m_OldPoint;

      CellOperation* doOp = new mitk::CellOperation(OpMOVECELL, m_CurrentCellId, vector);
	    if ( m_UndoEnabled )
      {
        Vector3D vectorBack = m_MovementStart - newPoint;
        CellOperation* undoOp = new mitk::CellOperation(OpMOVECELL, m_CurrentCellId, vectorBack);
        OperationEvent *operationEvent = new OperationEvent(m_DataTreeNode->GetData(),
                                            							  doOp, undoOp,
							                                              objectEventId, groupEventId);
        m_UndoController->SetOperationEvent(operationEvent);
      }
		  //execute the Operation
		  m_DataTreeNode->GetData()->ExecuteOperation(doOp);
    }
    ok = true;
    break;
  case AcREMOVE:
    ok = true;
    break;
  default:
    return Superclass::ExecuteAction(action, stateEvent, objectEventId, groupEventId);
  }
  return ok;
}

