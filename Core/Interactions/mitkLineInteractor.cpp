#include "mitkLineInteractor.h"
#include "mitkStatusBar.h"
#include <mitkInteractionConst.h>
#include <mitkLineOperation.h>
#include <mitkPointOperation.h>
#include <mitkPositionEvent.h>
#include <mitkStateTransitionOperation.h>
#include <mitkDataTreeNode.h>
#include <mitkMesh.h>
#include <mitkAction.h>
#include <mitkEventMapper.h>
#include <mitkLineOperation.h>
#include <mitkProperties.h>


mitk::LineInteractor::LineInteractor(const char * type, DataTreeNode* dataTreeNode)
: HierarchicalInteractor(type, dataTreeNode), m_CurrentCellId(0), m_CurrentLineId(0)
{
    m_PointInteractor = new mitk::PointInteractor("pointinteractor",dataTreeNode);
    this->AddInteractor((Interactor::Pointer)m_PointInteractor);
    m_LastPoint.Fill(0);
}

mitk::LineInteractor::~LineInteractor()
{
    //delete m_PointInteractor;
}

void mitk::LineInteractor::DeselectAllLines(int objectEventId, int groupEventId)
{
  mitk::Mesh* mesh = dynamic_cast<mitk::Mesh*>(m_DataTreeNode->GetData());
	if (mesh == NULL)
		return;

  //find the selected cell
  mitk::Mesh::DataType *itkMesh = mesh->GetMesh(); 
  mitk::Mesh::CellDataIterator cellDataIt;
  mitk::Mesh::CellDataIterator cellDataEnd;
  cellDataIt = itkMesh->GetCellData()->Begin();
  cellDataEnd = itkMesh->GetCellData()->End();
  while (cellDataIt != cellDataEnd)
  {
    if(cellDataIt->Value().selected)
      break;
    ++cellDataIt;
  }
  //then deselect all selected lines
  if (cellDataIt != cellDataEnd)
  {
    mitk::PointSet::SelectedLinesType selectedLines = cellDataIt->Value().selectedLines;
    for (int i = 0; i < selectedLines.size(); i++)
		{
      mitk::LineOperation* doOp = new mitk::LineOperation(OpDESELECTLINE, cellDataIt->Index(), -1, -1, selectedLines[i]);
			if (m_UndoEnabled)
			{
				mitk::LineOperation* undoOp = new mitk::LineOperation(OpSELECTLINE, cellDataIt->Index(), -1, -1, selectedLines[i]);
				OperationEvent *operationEvent = new OperationEvent(mesh,
																	doOp, undoOp,
																	objectEventId, groupEventId);
				m_UndoController->SetOperationEvent(operationEvent);
			}
			mesh->ExecuteOperation(doOp);
		}
  }
}

bool mitk::LineInteractor::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent, int objectEventId, int groupEventId)
{
  bool ok = false;//for return type bool

  //checking corresponding Data; has to be a PointSet or a subclass
	mitk::Mesh* mesh = dynamic_cast<mitk::Mesh*>(m_DataTreeNode->GetData());
	if (mesh == NULL)
		return false;

  //for reading on the points, Id's etc
  mitk::Mesh::DataType *itkpointSet = mesh->GetMesh();
  mitk::Mesh::PointsContainer *points = itkpointSet->GetPoints();
  
  switch (action->GetActionId())
	{
  case AcDONOTHING:
    ok = true;
  break;
  //case AcTRANSMITEVENT:
  //  {
  //    //due to the use of guards-states the eventId can be changed from original to internal EventIds e.g. StYES.
  //    //so we have remap the event and transmitt the original event with proper id
  //    ok = m_PointInteractor->HandleEvent(mitk::EventMapper::RefreshStateEvent(const_cast<StateEvent*>(stateEvent)), objectEventId, groupEventId );
  //    //check the state of the machine and according to that change the state/mode of this statemachine
  //    int mode = m_PointInteractor->GetMode();
  //    if (mode == mitk::Interactor::SMSELECTED)
  //      this->SetMode(mitk::Interactor::SMSUBSELECTED);
  //    else 
  //      this->SetMode(mitk::Interactor::SMDESELECTED);
  //  }
	 // break;
  case AcADDLINE:
    {
      Operation* doOp = new mitk::Operation(OpADDLINE);
		  if (m_UndoEnabled)
		  {
		  	Operation* undoOp = new mitk::Operation(OpDELETELINE);
			  OperationEvent *operationEvent = new OperationEvent(mesh,
				  													doOp, undoOp,
					  												objectEventId, groupEventId);
			  m_UndoController->SetOperationEvent(operationEvent);
		  }
		  //execute the Operation
      mesh->ExecuteOperation(doOp );
    }
    ok = true;
    break;
  case AcDESELECTALL:
  {
    this->DeselectAllLines(objectEventId, groupEventId);//undo-supported able deselect of all points in the DataList
    ok = true;
  }
  break;
  case AcCHECKPOINT:
    //check if a point is hit
    {
      mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		  if (posEvent != NULL)
      {
        mitk::Point3D worldPoint = posEvent->GetWorldPosition();
        
        int PRECISION = 1;
        mitk::IntProperty *precision = dynamic_cast<IntProperty*>(action->GetProperty("PRECISION"));
        if (precision != NULL)
        {
          PRECISION = precision->GetValue();
        }

			  int position = mesh->SearchPoint(worldPoint, PRECISION);
			  if (position>=0)//found a point near enough to the given point
			  {
          worldPoint = mesh->GetPoint(position);//get that point, the one meant by the user!
				  mitk::Point2D displPoint;
          displPoint[0] = worldPoint[0];
          displPoint[1] = worldPoint[1];
          //new Event with information YES and with the correct point
          mitk::PositionEvent const* newPosEvent = new mitk::PositionEvent(posEvent->GetSender(), posEvent->GetType(), posEvent->GetButton(), posEvent->GetButtonState(), posEvent->GetKey(), displPoint, worldPoint);
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(StYES, newPosEvent);
          //call HandleEvent to leave the guard-state
          this->HandleEvent( newStateEvent, objectEventId, groupEventId );
				  ok = true;
			  }
			  else
			  {
				  //new Event with information NO
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(StNO, posEvent);
          this->HandleEvent(newStateEvent, objectEventId, groupEventId );
				  ok = true;
			  }
		  }
    }
    break;
  case AcCHECKELEMENT:
    //checks if the position of the mouse lies over a line
    {
      mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		  if (posEvent != NULL)
      {
        mitk::Point3D worldPoint = posEvent->GetWorldPosition();

        unsigned long lineId, cellId;

        int PRECISION = 1;
        mitk::IntProperty *precision = dynamic_cast<IntProperty*>(action->GetProperty("PRECISION"));
        if (precision != NULL)
        {
          PRECISION = precision->GetValue();
        }
        
        bool found = mesh->SearchLine(worldPoint, PRECISION, lineId, cellId);
			  if (found)//found a point near enough to the given point
			  {
          m_CurrentLineId = lineId;
          m_CurrentCellId = cellId;
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(StYES, posEvent);
          //call HandleEvent to leave the guard-state
          this->HandleEvent( newStateEvent, objectEventId, groupEventId );
				  ok = true;
  			}
	  		else //not found
		  	{
				  //new Event with information NO
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(StNO, posEvent);
          this->HandleEvent(newStateEvent, objectEventId, groupEventId );
				  ok = true;
			  }
		  }
    }
    break;
  case AcCHECKSELECTED:
    //check if the selected line is still hit
    {
      mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
		  if (posEvent != NULL)
      {
        mitk::Point3D worldPoint = posEvent->GetWorldPosition();

        unsigned long lineId, cellId;

        int PRECISION = 1;
        mitk::IntProperty *precision = dynamic_cast<IntProperty*>(action->GetProperty("PRECISION"));
        if (precision != NULL)
        {
          PRECISION = precision->GetValue();
        }

        bool found = mesh->SearchLine(worldPoint, PRECISION, lineId, cellId);
        if (found && 
            m_CurrentCellId == cellId &&
            m_CurrentLineId == lineId )//found the same line again
			  {
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(StYES, posEvent);
          //call HandleEvent to leave the guard-state
          this->HandleEvent( newStateEvent, objectEventId, groupEventId );
				  ok = true;
  			}
	  		else //not found
		  	{
				  //new Event with information NO
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(StNO, posEvent);
          this->HandleEvent(newStateEvent, objectEventId, groupEventId );
				  ok = true;
			  }
		  }    
    }
    break;
  case AcREMOVE:
    //removes a line. Not to use if other interactors are influenced by this operation!
    ok = true;
    break;
  case AcSELECT :
    //select the current Line
    {
      LineOperation* doOp = new mitk::LineOperation(OpSELECTLINE, m_CurrentCellId, 0, 0, m_CurrentLineId);
      //Undo
			if (m_UndoEnabled)	//write to UndoMechanism
      {
				LineOperation* undoOp = new mitk::LineOperation(OpDESELECTLINE, m_CurrentCellId, 0, 0, m_CurrentLineId);
				OperationEvent *operationEvent = new OperationEvent(mesh,
																	doOp, undoOp,
																	objectEventId, groupEventId);
				m_UndoController->SetOperationEvent(operationEvent);
			}

			//execute the Operation
			mesh->ExecuteOperation(doOp);
			ok = true;
    }
    break;
  case AcDESELECT:
    //deselects the line 
  {
     LineOperation* doOp = new mitk::LineOperation(OpDESELECTLINE, m_CurrentCellId, 0, 0, m_CurrentLineId);
      //Undo
			if (m_UndoEnabled)	//write to UndoMechanism
      {
				LineOperation* undoOp = new mitk::LineOperation(OpSELECTLINE, m_CurrentCellId, 0, 0, m_CurrentLineId);
				OperationEvent *operationEvent = new OperationEvent(mesh,
																	doOp, undoOp,
																	objectEventId, groupEventId);
				m_UndoController->SetOperationEvent(operationEvent);
			}

			//execute the Operation
			mesh->ExecuteOperation(doOp);
			ok = true;
  }
  break;
  case AcINITMOVEMENT:
  {
     mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
     if (posEvent == NULL)
      return false;

    //start of the Movement is stored to calculate the undoKoordinate in FinishMovement
    m_LastPoint = posEvent->GetWorldPosition();
    m_TempPoint = m_LastPoint;
    ok = true;
    break;
  }
  break;
  case AcMOVE:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
    if (posEvent == NULL)
      return false;

    mitk::Point3D newPoint;
    newPoint = posEvent->GetWorldPosition();

    int idA, idB;
    Vector3D vector;
    
    vector = newPoint - m_TempPoint;
    m_TempPoint = newPoint;

    //get the ids of the points
    bool found = mesh->GetPointIds(m_CurrentCellId, m_CurrentLineId, idA, idB);
    if (found)
    {
      LineOperation* doOp = new mitk::LineOperation(OpMOVELINE, m_CurrentCellId, vector, idA, idB, m_CurrentLineId);
      //execute the Operation
      //here no undo is stored, because the movement-steps aren't interesting. only the start and the end is interisting to store for undo.
      m_DataTreeNode->GetData()->ExecuteOperation(doOp);
      ok = true;
    }
    else
      ok = false;
  }
  break;
  case AcFINISHMOVEMENT:
  {
    mitk::PositionEvent const *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
    if (posEvent == NULL)
      return false;

    //finish the movement:
    // set undo-information and move it to the last position.
    mitk::Point3D newPoint = posEvent->GetWorldPosition();

    int idA, idB;
    Vector3D vector;
    
    vector = m_TempPoint - newPoint;
    m_TempPoint.Fill(0);//not needed any more

    //get the ids of the points
    bool found = mesh->GetPointIds(m_CurrentCellId, m_CurrentLineId, idA, idB);
    if (found)
    {
      LineOperation* doOp = new mitk::LineOperation(OpMOVELINE, m_CurrentCellId, vector, idA, idB, m_CurrentLineId);
      if ( m_UndoEnabled )
      {
        LineOperation* undoOp = new mitk::LineOperation(OpMOVELINE, m_CurrentCellId, m_LastPoint-newPoint, idA, idB, m_CurrentLineId);
        OperationEvent *operationEvent = new OperationEvent(m_DataTreeNode->GetData(),
                                                          doOp, undoOp,
                                                          objectEventId, groupEventId);
        m_UndoController->SetOperationEvent(operationEvent);
      }
      //execute the Operation
      //here no undo is stored, because the movement-steps aren't interesting. only the start and the end is interisting to store for undo.
      m_DataTreeNode->GetData()->ExecuteOperation(doOp);
      ok = true;
    }
    else
      ok = false;
  }
  break;
  default:
    return Superclass::ExecuteAction(action, stateEvent, objectEventId, groupEventId);
  }
  return ok;
}
