/*===================================================================

 The Medical Imaging Interaction Toolkit (MITK)

 Copyright (c) German Cancer Research Center,
 Division of Medical and Biological Informatics.
 All rights reserved.

 This software is distributed WITHOUT ANY WARRANTY; without
 even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.

 See LICENSE.txt or http://www.mitk.org for details.

 ===================================================================*/

#include "mitkPointSetDataInteractor.h"
#include "mitkMouseMoveEvent.h"

#include "mitkOperationEvent.h"
#include <mitkPointOperation.h>
#include "mitkInteractionConst.h" // TODO: refactor file
#include "mitkRenderingManager.h"
#include "mitkInternalEvent.h"
//
#include "mitkDispatcher.h"
#include "mitkBaseRenderer.h"

#include "mitkUndoController.h"

void mitk::PointSetDataInteractor::ConnectActionsAndFunctions()
{
  // Condition which is evaluated before transition is taken
  // following actions in the statemachine are only executed if it returns TRUE
  CONNECT_CONDITION("isoverpoint", CheckSelection);
  CONNECT_FUNCTION("addpoint", AddPoint);
  CONNECT_FUNCTION("selectpoint", SelectPoint);
  CONNECT_FUNCTION("unselect", UnSelectPointAtPosition);
  CONNECT_FUNCTION("unselectAll", UnSelectAll);
  CONNECT_FUNCTION("initMove", InitMove);
  CONNECT_FUNCTION("movePoint", MovePoint);
  CONNECT_FUNCTION("finishMovement", FinishMove);
  CONNECT_FUNCTION("removePoint", RemovePoint);
}

bool mitk::PointSetDataInteractor::AddPoint(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent)
{
  unsigned int timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());
  ScalarType timeInMs = interactionEvent->GetSender()->GetTime();

  // disallow adding of new points if maximum number of points is reached
  if (m_MaxNumberOfPoints > 1 &&  m_PointSet->GetSize(timeStep) >= m_MaxNumberOfPoints)
  {
    return false;
  }
  // To add a point the minimal information is the position, this method accepts all InteractionsPositionEvents
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    // 1) Undo/Redo Supports Grouping of Operations that are undone as a set,
    // this statement indicates that a new Operation starts here
    mitk::OperationEvent::IncCurrObjectEventId();

    mitk::Point3D itkPoint = positionEvent->GetPositionInWorld();

    this->UnselectAll( timeStep, timeInMs);

    int lastPosition = 0;
    mitk::PointSet::PointsIterator it, end;
    it = m_PointSet->Begin();
    end = m_PointSet->End();
    while( it != end )
    {
      if (!m_PointSet->IndexExists(lastPosition,timeStep))
        break;
      ++it;
      ++lastPosition;
    }

    // Insert a Point to the PointSet
    // 2) Create the Operation inserting the point

    PointOperation* doOp = new mitk::PointOperation(OpINSERT,timeInMs, itkPoint, lastPosition);

    // 3) If Undo is enabled, also create the inverse Operation
    if (m_UndoEnabled)
    {
      PointOperation *undoOp = new mitk::PointOperation( OpREMOVE,timeInMs, itkPoint, lastPosition);
      // 4) Do and Undo Operations are combined in an Operation event which also contains the target of the operations (here m_PointSet)
      OperationEvent *operationEvent = new OperationEvent(m_PointSet, doOp, undoOp, "Add point");
      // 5) Store the Operation in the UndoController
      m_UndoController->SetOperationEvent(operationEvent);
    }

    // 6) Execute the Operation performs the actual insertion of the point into the PointSet
    m_PointSet->ExecuteOperation(doOp);

    // 7) If Undo is not enabled the Do-Operation is to be dropped to prevent memory leaks.
    if ( !m_UndoEnabled )
      delete doOp;

    // Request update
    interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();

    // Check if points form a closed contour now, if so fire an InternalEvent
    IsClosedContour(stateMachineAction, interactionEvent);

    if (m_MaxNumberOfPoints > 0 &&  m_PointSet->GetSize(timeStep) >= m_MaxNumberOfPoints)
    {
      // Signal that DataNode is fully filled
      this->NotifyResultReady();
      // Send internal event that can be used by StateMachines to switch in a different state
      InternalEvent::Pointer event = InternalEvent::New(NULL, this, "MaximalNumberOfPoints");
      positionEvent->GetSender()->GetDispatcher()->QueueEvent(event.GetPointer());
    }

    return true;
  }
  else
  {
    return false;
  }
}

bool mitk::PointSetDataInteractor::SelectPoint(StateMachineAction*, InteractionEvent* interactionEvent)
{
  unsigned int timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());
  ScalarType timeInMs = interactionEvent->GetSender()->GetTime();

  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    Point3D point = positionEvent->GetPositionInWorld();
    // iterate over point set and check if it contains a point close enough to the pointer to be selected
    int index = GetPointIndexByPosition(point, timeStep);
    if (index != -1)
    {
      //first deselect the other points
      //undoable deselect of all points in the DataList
      this->UnselectAll( timeStep, timeInMs);

      PointOperation* doOp = new mitk::PointOperation(OpSELECTPOINT,timeInMs, point, index);

      //Undo
      if (m_UndoEnabled)
      {
        PointOperation* undoOp = new mitk::PointOperation(OpDESELECTPOINT,timeInMs,point, index);
        OperationEvent *operationEvent = new OperationEvent(m_PointSet, doOp, undoOp);
        m_UndoController->SetOperationEvent(operationEvent);
      }

      //execute the Operation
      m_PointSet->ExecuteOperation(doOp);

      if ( !m_UndoEnabled )
        delete doOp;

      interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();
      return true;
    }
  }
  return false;
}

mitk::PointSetDataInteractor::PointSetDataInteractor() :
  m_MaxNumberOfPoints(0),m_SelectionAccuracy(3.5)
{
}

mitk::PointSetDataInteractor::~PointSetDataInteractor()
{
}

bool mitk::PointSetDataInteractor::RemovePoint(StateMachineAction*, InteractionEvent* interactionEvent)
{
  unsigned int timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());
  ScalarType timeInMs = interactionEvent->GetSender()->GetTime();

  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    mitk::OperationEvent::IncCurrObjectEventId();

    mitk::Point3D itkPoint = positionEvent->GetPositionInWorld();

    //search the point in the list
    int position = m_PointSet->SearchPoint( itkPoint , m_SelectionAccuracy, timeStep);
    if (position>=0)//found a point
    {
      PointSet::PointType pt = m_PointSet->GetPoint(position, timeStep);
      itkPoint[0] = pt[0];
      itkPoint[1] = pt[1];
      itkPoint[2] = pt[2];

      PointOperation* doOp = new mitk::PointOperation(OpREMOVE,timeInMs, itkPoint, position);
      if (m_UndoEnabled)  //write to UndoMechanism
      {
        PointOperation* undoOp = new mitk::PointOperation(OpINSERT,timeInMs, itkPoint, position);
        OperationEvent *operationEvent = new OperationEvent(m_PointSet, doOp, undoOp, "Remove point");
        m_UndoController->SetOperationEvent(operationEvent);
      }
      //execute the Operation
      m_PointSet->ExecuteOperation(doOp);

      if ( !m_UndoEnabled )
        delete doOp;

      /*now select the point "position-1",
      and if it is the first in list,
      then continue at the last in list*/
      //only then a select of a point is possible!
      if (m_PointSet->GetSize( timeStep ) > 0)
      {
        this->SelectPoint( m_PointSet->Begin( timeStep )->Index(), timeStep, timeInMs );
      }
    }
    interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();
  }
  else
  {
    return false;
  }
  return true;
}

bool mitk::PointSetDataInteractor::IsClosedContour(StateMachineAction*, InteractionEvent* interactionEvent)
{
  unsigned int timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());

  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    Point3D point = positionEvent->GetPositionInWorld();
    // iterate over point set and check if it contains a point close enough to the pointer to be selected
    if (GetPointIndexByPosition(point, timeStep) != -1 && m_PointSet->GetSize(timeStep) >= 3)
    {
      InternalEvent::Pointer event = InternalEvent::New(NULL, this, "ClosedContour");
      positionEvent->GetSender()->GetDispatcher()->QueueEvent(event.GetPointer());
      return true;
    }
  }
  return false;
}

bool mitk::PointSetDataInteractor::MovePoint(StateMachineAction* stateMachineAction, InteractionEvent* interactionEvent)
{
  unsigned int timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());
  ScalarType timeInMs = interactionEvent->GetSender()->GetTime();
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    IsClosedContour(stateMachineAction, interactionEvent);
    mitk::Point3D newPoint, resultPoint;
    newPoint = positionEvent->GetPositionInWorld();

    // search the elements in the list that are selected then calculate the
    // vector, because only with the vector we can move several elements in
    // the same direction
    //   newPoint - lastPoint = vector
    // then move all selected and set the lastPoint = newPoint.
    // then add all vectors to a summeryVector (to be able to calculate the
    // startpoint for undoOperation)
    mitk::Vector3D dirVector = newPoint - m_LastPoint;

    //sum up all Movement for Undo in FinishMovement
    m_SumVec = m_SumVec + dirVector;

    mitk::PointSet::PointsIterator it, end;
    it = m_PointSet->Begin();
    end = m_PointSet->End();
    while( it != end )
    {
      int position = it->Index();
      if ( m_PointSet->GetSelectInfo(position, timeStep) )//if selected
      {
        PointSet::PointType pt = m_PointSet->GetPoint(position, timeStep);
        mitk::Point3D sumVec;
        sumVec[0] = pt[0];
        sumVec[1] = pt[1];
        sumVec[2] = pt[2];
        resultPoint = sumVec + dirVector;
        PointOperation* doOp = new mitk::PointOperation(OpMOVE,timeInMs, resultPoint, position);
        //execute the Operation
        //here no undo is stored, because the movement-steps aren't interesting.
        // only the start and the end is interisting to store for undo.
        m_PointSet->ExecuteOperation(doOp);
        delete doOp;
      }
      ++it;
    }
    m_LastPoint = newPoint;//for calculation of the direction vector
    // Update the display
    interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();
    IsClosedContour(stateMachineAction,interactionEvent);
    return true;
  }
  else
  {
    return false;
  }
}


bool mitk::PointSetDataInteractor::UnSelectPointAtPosition(StateMachineAction*, InteractionEvent* interactionEvent)
{
  unsigned int timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());
  ScalarType timeInMs = interactionEvent->GetSender()->GetTime();

  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    Point3D point = positionEvent->GetPositionInWorld();
    // iterate over point set and check if it contains a point close enough to the pointer to be selected
    int index = GetPointIndexByPosition(point, timeStep);
    // here it is ensured that we don't switch from one point being selected to another one being selected,
    // without accepting the unselect of the current point
    if (index != -1)
    {
      PointOperation* doOp = new mitk::PointOperation(OpDESELECTPOINT,timeInMs, point, index);
      if (m_UndoEnabled)  //write to UndoMechanism
      {
        PointOperation* undoOp = new mitk::PointOperation(OpSELECTPOINT,timeInMs, point, index);
        OperationEvent *operationEvent = new OperationEvent(m_PointSet, doOp, undoOp);
        m_UndoController->SetOperationEvent(operationEvent);
      }
      //execute the Operation
      m_PointSet->ExecuteOperation(doOp);

      if ( !m_UndoEnabled )
        delete doOp;

      return true;

    }
  }
  return false;
}

bool mitk::PointSetDataInteractor::UnSelectAll(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
  unsigned int timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());
  ScalarType timeInMs = interactionEvent->GetSender()->GetTime();

  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {

    Point3D positioninWorld = positionEvent->GetPositionInWorld();
    PointSet::PointsContainer::Iterator it, end;


    PointSet::DataType *itkPointSet = m_PointSet->GetPointSet( timeStep );

    end = itkPointSet->GetPoints()->End();

    for (it = itkPointSet->GetPoints()->Begin(); it != end; it++)
    {
      int position = it->Index();
      //then declare an operation which unselects this point;
      //UndoOperation as well!
      if ( m_PointSet->GetSelectInfo(position,timeStep) )
      {

        float distance = sqrt(positioninWorld.SquaredEuclideanDistanceTo(m_PointSet->GetPoint(position, timeStep)));
        if (distance > m_SelectionAccuracy)
        {
          mitk::Point3D noPoint;
          noPoint.Fill( 0 );
          mitk::PointOperation* doOp = new mitk::PointOperation(OpDESELECTPOINT,timeInMs,  noPoint, position);

          if ( m_UndoEnabled )
          {
            mitk::PointOperation* undoOp = new mitk::PointOperation(OpSELECTPOINT, timeInMs,  noPoint, position);
            OperationEvent *operationEvent = new OperationEvent( m_PointSet, doOp, undoOp );

            m_UndoController->SetOperationEvent( operationEvent );
          }

          m_PointSet->ExecuteOperation( doOp );

          if ( !m_UndoEnabled )
            delete doOp;
        }
      }

    }
  }
  else
  {
    this->UnselectAll(timeStep,timeInMs);
  }

  return true;
}

bool mitk::PointSetDataInteractor::UpdatePointSet(mitk::StateMachineAction*, mitk::InteractionEvent*)
{
  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(this->GetDataNode()->GetData());
  if ( pointSet == NULL )
  {
    return false;
    MITK_ERROR << "PointSetDataInteractor:: No valid point set .";
  }

  m_PointSet = pointSet;
  return true;
}

bool mitk::PointSetDataInteractor::Abort(StateMachineAction*, InteractionEvent* interactionEvent)
{
  InternalEvent::Pointer event = InternalEvent::New(NULL, this, IntDeactivateMe);
  interactionEvent->GetSender()->GetDispatcher()->QueueEvent(event.GetPointer());
  return true;
}


/*
 * Check whether the DataNode contains a pointset, if not create one and add it.
 */
void mitk::PointSetDataInteractor::DataNodeChanged()
{
  if (GetDataNode().IsNotNull())
  {
    PointSet* points = dynamic_cast<PointSet*>(GetDataNode()->GetData());
    if (points == NULL)
    {
      m_PointSet = PointSet::New();
      GetDataNode()->SetData(m_PointSet);
    }
    else
    {
      m_PointSet = points;
    }
    // load config file parameter: maximal number of points
    mitk::PropertyList::Pointer properties = GetAttributes();
    std::string strNumber;
    if (properties->GetStringProperty("MaxPoints", strNumber))
    {
      m_MaxNumberOfPoints = atoi(strNumber.c_str());
    }
  }
}

bool mitk::PointSetDataInteractor::InitMove(StateMachineAction*, InteractionEvent* interactionEvent)
{

  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);

  if (positionEvent == NULL)
    return false;

  mitk::OperationEvent::IncCurrObjectEventId();

  // start of the Movement is stored to calculate the undoKoordinate
  // in FinishMovement
  m_LastPoint = positionEvent->GetPositionInWorld();

  // initialize a value to calculate the movement through all
  // MouseMoveEvents from MouseClick to MouseRelease
  m_SumVec.Fill(0);

  GetDataNode()->SetProperty("contourcolor", ColorProperty::New(1.0, 1.0, 1.0));
  return true;
}

bool mitk::PointSetDataInteractor::FinishMove(StateMachineAction*, InteractionEvent* interactionEvent)
{

  unsigned int timeStep = interactionEvent->GetSender()->GetTimeStep(GetDataNode()->GetData());
  ScalarType timeInMs = interactionEvent->GetSender()->GetTime();

  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);

  if (positionEvent != NULL)
  {
    //finish the movement:
    //the final point is m_LastPoint
    //m_SumVec stores the movement in a vector
    //the operation would not be necessary, but we need it for the undo Operation.
    //m_LastPoint is for the Operation
    //the point for undoOperation calculates from all selected
    //elements (point) - m_SumVec

    //search all selected elements and move them with undo-functionality.

    mitk::PointSet::PointsIterator it, end;
    it = m_PointSet->Begin();
    end = m_PointSet->End();
    while( it != end )
    {
      int position = it->Index();
      if ( m_PointSet->GetSelectInfo(position, timeStep) )//if selected
      {
        PointSet::PointType pt = m_PointSet->GetPoint(position, timeStep);
        Point3D itkPoint;
        itkPoint[0] = pt[0];
        itkPoint[1] = pt[1];
        itkPoint[2] = pt[2];
        PointOperation* doOp = new mitk::PointOperation(OpMOVE,timeInMs,itkPoint, position);

        if ( m_UndoEnabled )//&& (posEvent->GetType() == mitk::Type_MouseButtonRelease)
        {
          //set the undo-operation, so the final position is undo-able
          //calculate the old Position from the already moved position - m_SumVec
          mitk::Point3D undoPoint = ( itkPoint - m_SumVec );
          PointOperation* undoOp = new mitk::PointOperation(OpMOVE,timeInMs, undoPoint, position);
          OperationEvent *operationEvent =  new OperationEvent(m_PointSet, doOp, undoOp, "Move point");
          m_UndoController->SetOperationEvent(operationEvent);
        }
        //execute the Operation
        m_PointSet->ExecuteOperation(doOp);

        if ( !m_UndoEnabled )
          delete doOp;

      }
      ++it;
    }

    // Update the display
    interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();
    OperationEvent::IncCurrGroupEventId();
  }
  else
  {return false;
  }
  this->NotifyResultReady();
  return true;
}


void mitk::PointSetDataInteractor::SetAccuracy(float accuracy)
{
  m_SelectionAccuracy = accuracy;
}

void mitk::PointSetDataInteractor::SetMaxPoints(unsigned int maxNumber)
{
  m_MaxNumberOfPoints = maxNumber;
}

int mitk::PointSetDataInteractor::GetPointIndexByPosition(Point3D position, int time, float accuracy)
{
  // iterate over point set and check if it contains a point close enough to the pointer to be selected
  PointSet* points = dynamic_cast<PointSet*>(GetDataNode()->GetData());
  int index = -1;
  if (points == NULL)
  {
    return index;
  }
  PointSet::PointsContainer* pointsContainer = points->GetPointSet(time)->GetPoints();

  float minDistance = m_SelectionAccuracy;
  if (accuracy != -1 )
    minDistance = accuracy;


  for (PointSet::PointsIterator it = pointsContainer->Begin(); it != pointsContainer->End(); it++)
  {
    float distance = sqrt(position.SquaredEuclideanDistanceTo(points->GetPoint(it->Index(), time)));
    if (distance < minDistance) // if several points fall within the margin, choose the one with minimal distance to position
    {
      index = it->Index();
    }
  }
  return index;
}

bool mitk::PointSetDataInteractor::CheckSelection(const mitk::InteractionEvent *interactionEvent)
{
  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    int timeStep = positionEvent->GetSender()->GetTimeStep();
    Point3D point = positionEvent->GetPositionInWorld();
    // iterate over point set and check if it contains a point close enough to the pointer to be selected
    int index = GetPointIndexByPosition(point, timeStep);
    if (index != -1)
      return true;
  }
  return false;
}



void mitk::PointSetDataInteractor::UnselectAll(unsigned int timeStep, ScalarType timeInMs)
{
  mitk::PointSet *pointSet = dynamic_cast<mitk::PointSet*>( GetDataNode()->GetData() );
  if ( pointSet == NULL )
  {
    return;
  }

  mitk::PointSet::DataType *itkPointSet = pointSet->GetPointSet( timeStep );
  if ( itkPointSet == NULL )
  {
    return;
  }

  mitk::PointSet::PointsContainer::Iterator it, end;
  end = itkPointSet->GetPoints()->End();

  for (it = itkPointSet->GetPoints()->Begin(); it != end; it++)
  {
    int position = it->Index();
    PointSet::PointDataType pointData = {0, false, PTUNDEFINED};
    itkPointSet->GetPointData( position, &pointData );

    //then declare an operation which unselects this point;
    //UndoOperation as well!
    if ( pointData.selected )
    {
      mitk::Point3D noPoint;
      noPoint.Fill( 0 );
      mitk::PointOperation *doOp = new mitk::PointOperation(OpDESELECTPOINT,timeInMs, noPoint, position);

      if ( m_UndoEnabled )
      {
        mitk::PointOperation *undoOp =
            new mitk::PointOperation(OpSELECTPOINT, timeInMs, noPoint, position);
        OperationEvent *operationEvent = new OperationEvent( pointSet, doOp, undoOp );

        m_UndoController->SetOperationEvent( operationEvent );
      }

      pointSet->ExecuteOperation( doOp );

      if ( !m_UndoEnabled )
        delete doOp;
    }
  }
}



void mitk::PointSetDataInteractor::SelectPoint(int position, unsigned int timeStep, ScalarType timeInMS)
{
  mitk::PointSet *pointSet = dynamic_cast< mitk::PointSet * >(
        this->GetDataNode()->GetData() );

  //if List is empty, then no selection of a point can be done!
  if ( (pointSet == NULL) || (pointSet->GetSize( timeStep ) <= 0) )
  {
    return;
  }

  //dummyPoint... not needed anyway
  mitk::Point3D noPoint;
  noPoint.Fill(0);

  mitk::PointOperation *doOp = new mitk::PointOperation( OpSELECTPOINT,timeInMS, noPoint, position);

  if ( m_UndoEnabled )
  {
    mitk::PointOperation* undoOp = new mitk::PointOperation(OpDESELECTPOINT,timeInMS, noPoint, position);

    OperationEvent *operationEvent = new OperationEvent(pointSet, doOp, undoOp);
    m_UndoController->SetOperationEvent(operationEvent);
  }

  pointSet->ExecuteOperation( doOp );

  if ( !m_UndoEnabled )
    delete doOp;

}
