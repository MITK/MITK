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
#include <vtkTransform.h>


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
  float returnJurisdiction = 0;
  
  mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
  //checking if a keyevent can be handled:
  if (posEvent == NULL)
  {
    //check, if the current state has a transition waiting for that key event.
    if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
    {
      return 0.5;
    }
    else
    {
      return 0;
    }
  }

  //Mouse event handling:
  //on MouseMove do nothing! reimplement if needed differently
  if (stateEvent->GetEvent()->GetType() == mitk::Type_MouseMove)
  {
    return 0;
  }

  //check on the right data-type
  mitk::Mesh* mesh = dynamic_cast<mitk::Mesh*>(m_DataTreeNode->GetData());
	if (mesh == NULL)
		return false;


  //go throgh all cells and get the BoundingBox:
  Mesh::ConstCellIterator cellIt = mesh->GetMesh()->GetCells()->Begin();
  Mesh::ConstCellIterator cellEnd = mesh->GetMesh()->GetCells()->End();
  while( cellIt != cellEnd )
  {

    Mesh::DataType::BoundingBoxPointer bBox = mesh->GetBoundingBoxFromCell( cellIt->Index() );
    if (bBox == NULL)
      return 0;
    
    //since we now have 3D picking in GlobalInteraction and all events send are DisplayEvents with 3D information,
    //we concentrate on 3D coordinates
    mitk::Point3D worldPoint = posEvent->GetWorldPosition();
    float p[3];
    itk2vtk(worldPoint, p);
    //transforming the Worldposition to local coordinatesystem
    m_DataTreeNode->GetData()->GetGeometry()->GetVtkTransform()->GetInverse()->TransformPoint(p, p);
    vtk2itk(p, worldPoint);

    //distance between center and point 
    mitk::BoundingBox::PointType center = bBox->GetCenter();
    returnJurisdiction = worldPoint.EuclideanDistanceTo(center);
    
    //now compared to size of boundingbox to get between 0 and 1;
    returnJurisdiction = returnJurisdiction/( (bBox->GetMaximum().EuclideanDistanceTo(bBox->GetMinimum() ) ) );
    
    //shall be 1 if short length to center
    returnJurisdiction = 1 - returnJurisdiction;

    //check if the given position lies inside the data-object
    if (bBox->IsInside(worldPoint))
    {
      //mapped between 0,5 and 1
      returnJurisdiction = 0.5 + (returnJurisdiction / 2);
    }
    else
    {
      //set it in range between 0 and 0.5
      returnJurisdiction = returnJurisdiction / 2;
    }
    ++cellIt;
  }


//now check all lower statemachines:
  InteractorListConstIter i = m_AllInteractors.begin();
  InteractorListConstIter end = m_AllInteractors.end();

  while ( i != end )
  {
    float currentJurisdiction = (*i)->CalculateJurisdiction( stateEvent );
    if (returnJurisdiction < currentJurisdiction)
      returnJurisdiction = currentJurisdiction;
    i++;
  }

  return returnJurisdiction;
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
  case AcDONOTHING:
    ok = true;
	  break;
  //case AcTRANSMITEVENT:
  //  {
  //    //due to the use of guards-states the eventId can be changed from original to internal EventIds e.g. EIDYES.
  //    //so we have remap the event and transmitt the original event with proper id
  //    ok = m_LineInteractor->HandleEvent(mitk::EventMapper::RefreshStateEvent(const_cast<StateEvent*>(stateEvent)) );
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
			  OperationEvent *operationEvent = new OperationEvent(mesh, doOp, undoOp);
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
			  OperationEvent *operationEvent = new OperationEvent(mesh, lineDoOp, lineUndoOp);
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
      unsigned long lineId = 0;
      unsigned long cellId = 0;
      int PRECISION = 1;
      mitk::IntProperty *precision = dynamic_cast<IntProperty*>(action->GetProperty("PRECISION"));
      if (precision != NULL)
      {
        PRECISION = precision->GetValue();
      }

      if(mesh->SearchLine(worldPoint, PRECISION, lineId, cellId))//line found
      {
        m_CurrentCellId = cellId;
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
        this->HandleEvent( newStateEvent );
        ok = true;
      }
      else
		  {
			  //new Event with information NO
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
        this->HandleEvent( newStateEvent );
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
        this->HandleEvent( newStateEvent );

      }
      else
      {
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
        this->HandleEvent(newStateEvent );
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
      unsigned long cellId = 0;
      
      //check wheather the mesh is hit.
      if (mesh->EvaluatePosition(point, cellId))
      {
        m_CurrentCellId = cellId;
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
        this->HandleEvent( newStateEvent );

      }
      else
      {
        m_CurrentCellId = 0;
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
        this->HandleEvent(newStateEvent );
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
			  OperationEvent *operationEvent = new OperationEvent(mesh, lineDoOp, lineUndoOp);
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
			    OperationEvent *operationEvent = new OperationEvent(mesh, lineDoOp, lineUndoOp);
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
        OperationEvent *operationEvent = new OperationEvent(m_DataTreeNode->GetData(), doOp, undoOp);
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
    return Superclass::ExecuteAction(action, stateEvent);
  }
  return ok;
}

