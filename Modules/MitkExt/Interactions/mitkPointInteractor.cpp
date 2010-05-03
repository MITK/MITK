/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPointInteractor.h"
#include <mitkPointOperation.h>
#include <mitkPositionEvent.h>
#include <mitkOperationEvent.h>
//#include "mitkStatusBar.h"
#include <mitkDataNode.h>
#include <mitkPointSet.h>
#include <mitkInteractionConst.h>
#include <mitkAction.h>
#include <mitkProperties.h>
#include <vtkLinearTransform.h>
#include <mitkUndoController.h>
#include <mitkStateEvent.h>
#include <mitkState.h>


mitk::PointInteractor::PointInteractor(const char * type, DataNode* dataTreeNode)
: Interactor(type, dataTreeNode), m_LastPosition(0)
{
  m_LastPoint.Fill(0);
}

mitk::PointInteractor::~PointInteractor()
{}

void mitk::PointInteractor::SelectPoint(int position)
{
  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(m_DataNode->GetData());
  if (pointSet == NULL)
    return;
  if (pointSet->GetSize()<=0)//if List is empty, then no select of a point can be done!
    return;

  mitk::Point3D noPoint;//dummyPoint... not needed anyway
  noPoint.Fill(0);
  mitk::PointOperation* doOp = new mitk::PointOperation(OpSELECTPOINT, noPoint, position);
  if (m_UndoEnabled)
  {
    mitk::PointOperation* undoOp = new mitk::PointOperation(OpDESELECTPOINT, noPoint, position);
    OperationEvent *operationEvent = new OperationEvent(pointSet, doOp, undoOp);
    m_UndoController->SetOperationEvent(operationEvent);
  }
  pointSet->ExecuteOperation(doOp);
}

void mitk::PointInteractor::DeselectAllPoints()
{
  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(m_DataNode->GetData());
  if (pointSet == NULL)
    return;

  mitk::PointSet::DataType *itkPointSet = pointSet->GetPointSet();
  mitk::PointSet::PointsContainer::Iterator it, end;
  end = itkPointSet->GetPoints()->End();

  for (it = itkPointSet->GetPoints()->Begin(); it != end; it++)
  {
    int position = it->Index();
    PointSet::PointDataType pointData = {0, false, PTUNDEFINED};
    itkPointSet->GetPointData(position, &pointData);
    if ( pointData.selected )//then declare an operation which unselects this point; UndoOperation as well!
    {
      mitk::Point3D noPoint;
      noPoint.Fill(0);

      mitk::PointOperation* doOp = new mitk::PointOperation(OpDESELECTPOINT, noPoint, position);
      if (m_UndoEnabled)
      {
        mitk::PointOperation* undoOp = new mitk::PointOperation(OpSELECTPOINT, noPoint, position);
        OperationEvent *operationEvent = new OperationEvent(pointSet, doOp, undoOp);
        m_UndoController->SetOperationEvent(operationEvent);
      }
      pointSet->ExecuteOperation(doOp);
    }
  }
}

float mitk::PointInteractor::CanHandleEvent(StateEvent const* stateEvent) const
//go through all points and check, if the given Point lies near a line
{
  float returnValue = 0;
  
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

  //if the event can be understood and if there is a transition waiting for that event
  if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
  {
    returnValue = 0.5;//it can be understood
  }

  //check on the right data-type
  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(m_DataNode->GetData());
  if (pointSet == NULL)
    return 0;


  //since we now have 3D picking in GlobalInteraction and all events send are DisplayEvents with 3D information,
  //we concentrate on 3D coordinates
  mitk::Point3D worldPoint = posEvent->GetWorldPosition();
  float p[3];
  itk2vtk(worldPoint, p);
  //transforming the Worldposition to local coordinatesystem
  m_DataNode->GetData()->GetGeometry()->GetVtkTransform()->GetInverse()->TransformPoint(p, p);
  vtk2itk(p, worldPoint);

  float distance = 5;
  int index = pointSet->SearchPoint(worldPoint, distance);
  if (index>-1)
    //how far away is the line from the point?
  {
    //get the point and calculate the jurisdiction out of it.
    mitk::PointSet::PointType point;
    pointSet->GetPointSet()->GetPoint(index, &point); 
    returnValue = point.EuclideanDistanceTo(worldPoint);

    //between 1 and 0.     1 if directly hit
    returnValue = 1 - ( returnValue / distance );
    if (returnValue<0 || returnValue>1)
    {
      itkWarningMacro("Difficulties in calculating Jurisdiction. Check PointInteractor");
      return 0;
    }
    
    //and now between 0,5 and 1
    returnValue = 0.5 + (returnValue / 2);

    return returnValue;
  }
  else //not found
  {
    return returnValue;
  }

}


bool mitk::PointInteractor::ExecuteAction( Action* action, mitk::StateEvent const* stateEvent )
{
  bool ok = false;//for return type bool
  
  //checking corresponding Data; has to be a PointSet or a subclass
  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(m_DataNode->GetData());
  if (pointSet == NULL)
    return false;

  //for reading on the points, Id's etc
  mitk::PointSet::DataType *itkPointSet = pointSet->GetPointSet();
  mitk::PointSet::PointsContainer *itkPoints = itkPointSet->GetPoints();

  /*Each case must watch the type of the event!*/
  switch (action->GetActionId())
  {
  case AcADDPOINT:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
    if (posEvent == NULL) 
      return false;

    mitk::Point3D itkPoint;
    itkPoint = posEvent->GetWorldPosition();

    //find the position, the point is to be added to: first entry with empty index.
    //if the Set is empty, then start with 0. if not empty, then take the first index not occupied
    int lastPosition = 0;
    if (!itkPoints->empty())
    {
      mitk::PointSet::PointsIterator it, end;
      it = itkPoints->Begin();
      end = itkPoints->End();
      while( it != end )
      {
        if (!itkPoints->IndexExists(lastPosition))
          break;
        ++it;
        ++lastPosition;
      }
    }
    
    PointOperation* doOp = new mitk::PointOperation(OpINSERT, itkPoint, lastPosition);
    if (m_UndoEnabled)
    {
      PointOperation* undoOp = new mitk::PointOperation(OpREMOVE, itkPoint, lastPosition);
      OperationEvent *operationEvent = new OperationEvent(pointSet, doOp, undoOp, "Add point");
      m_UndoController->SetOperationEvent(operationEvent);
    }
    pointSet->ExecuteOperation(doOp);
    ok = true;
    break;
  }
  case AcDESELECTALL:
    this->DeselectAllPoints();
    ok = true;
    break;
  case AcCHECKELEMENT :
    /*checking if the Point transmitted is close enough to one point. Then generate a new event with the point and let this statemaschine handle the event.*/
    {
      mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
      if (posEvent != NULL)
      {
        mitk::Point3D worldPoint = posEvent->GetWorldPosition();

        int PRECISION = 4;
        mitk::IntProperty *precision = dynamic_cast<IntProperty*>(action->GetProperty("PRECISION"));
        if (precision != NULL)
        {
          PRECISION = precision->GetValue();
        }

        int position = pointSet->SearchPoint(worldPoint, PRECISION);
        if (position>=0)//found a point near enough to the given point
        {
          m_LastPosition = position;//store it to be able to move the point
          worldPoint = pointSet->GetPoint(position);//get that point, the one meant by the user!
          mitk::Point2D displPoint;
          displPoint[0] = worldPoint[0];
          displPoint[1] = worldPoint[1];

          //new Event with information YES and with the correct point
          mitk::PositionEvent const* newPosEvent = new mitk::PositionEvent(posEvent->GetSender(), posEvent->GetType(), 
                                                                           posEvent->GetButton(), posEvent->GetButtonState(), posEvent->GetKey(), 
                                                                           displPoint, worldPoint);
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDYES, newPosEvent);
          //call HandleEvent to leave the guard-state
          this->HandleEvent( newStateEvent );
          ok = true;
        }
        else
        {
          //new Event with information NO
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, posEvent);
          this->HandleEvent( newStateEvent );
          ok = true;
        }
      }
      else //hardly used, due to 3dPicking of globalInteraction
      {
        mitk::DisplayPositionEvent const  *disPosEvent = dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());
        if (disPosEvent != NULL)
        {//2d Koordinates for 3D Interaction; return false to redo the last statechange
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, posEvent);
          this->HandleEvent( newStateEvent );
          ok = true;
        }
      }
    }
    break;
  case AcCHECKSELECTED:
    //check if the point that was hit before is still hit
    {
      mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
      if (posEvent != NULL)
      {
        mitk::Point3D worldPoint = posEvent->GetWorldPosition();

        int PRECISION = 4;
        mitk::IntProperty *precision = dynamic_cast<IntProperty*>(action->GetProperty("precision"));
        if (precision != NULL)
        {
          PRECISION = precision->GetValue();
        }

        int position = pointSet->SearchPoint(worldPoint, PRECISION);
        if (position <0)//no point found at the current mouseposition
        {
          //new Event with information NO
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, posEvent);
          this->HandleEvent( newStateEvent );
          ok = true;
          break;
        }

        //point found at the current mouseposition, so check if this point is the same like before
        unsigned int upos = (unsigned int) position;//comparison signed unsigned int
        if (upos == m_LastPosition)//found the same point again
        {
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDYES, posEvent);
          //call HandleEvent to leave the guard-state
          this->HandleEvent( newStateEvent );
          ok = true;
        }
        else
        {
          //new Event with information NO
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, posEvent);
          this->HandleEvent( newStateEvent );
          ok = true;
        }
      }
      else //hardly used, due to 3dPicking of globalInteraction
      {
        mitk::DisplayPositionEvent const  *disPosEvent = dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());
        if (disPosEvent != NULL)
        {//2d Koordinates for 3D Interaction; return false to redo the last statechange
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, posEvent);
          this->HandleEvent( newStateEvent );
          ok = true;
        }
      }    
    }
    break;
  case AcINITMOVEMENT:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
    if (posEvent == NULL)
      return false;
    
    //start of the Movement is stored to calculate the undoCoordinate in FinishMovement
    m_LastPoint = posEvent->GetWorldPosition();
    
    ok = true;
    break;
  }
  
  case AcMOVESELECTED://moves the point
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
    if (posEvent == NULL)
      return false;

    mitk::Point3D newPoint;
    newPoint = posEvent->GetWorldPosition();

    PointOperation* doOp = new mitk::PointOperation(OpMOVE, newPoint, m_LastPosition);
    //execute the Operation
    //here no undo is stored. only the start and the end is stored for undo.
    m_DataNode->GetData()->ExecuteOperation(doOp);
    ok = true;
    break;
 }

 case AcFINISHMOVEMENT:
 {
   mitk::PositionEvent const *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
   if (posEvent == NULL)
     return false;

    //finish the movement:
    // set undo-information and move it to the last position.
    mitk::Point3D newPoint;
    newPoint = posEvent->GetWorldPosition();

    PointOperation* doOp = new mitk::PointOperation(OpMOVE, newPoint, m_LastPosition);
    if ( m_UndoEnabled )
    {
      PointOperation* undoOp = new mitk::PointOperation(OpMOVE, m_LastPoint, m_LastPosition);
      OperationEvent *operationEvent = new OperationEvent( m_DataNode->GetData(), doOp, undoOp, "Move point");
      m_UndoController->SetOperationEvent(operationEvent);
    }
    //execute the Operation
    m_DataNode->GetData()->ExecuteOperation(doOp);

  //  //increase the GroupEventId, so that the raw-Undo goes to here
    //this->IncCurrGroupEventId();
    ok = true;
 }
 break;
case AcCHECKGREATERONE:
  //check if after deleteion the set will be empty
  {
    if (pointSet->GetSize()>1)
    {
      mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
      this->HandleEvent( newStateEvent );
      ok = true;
    }
    else 
    {
      mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
      this->HandleEvent( newStateEvent );
      ok = true;
    }
  }
  break;
case AcSELECTANOTHEROBJECT:
  //selects an other Point after one point has been removed
  {
    /*select the point "position-1", 
      and if it is the first in list, 
      then contine at the last in list*/
    if (pointSet->GetSize()>0)//only then a select of a point is possible!
    {
      if (m_LastPosition > 0)//not the first in list
      {
        this->SelectPoint( m_LastPosition-1 );
      }
      else//it was the first point in list, that was removed, so select the last in list
      {
        m_LastPosition = pointSet->GetSize()-1;//last in list
        SelectPoint( m_LastPosition );
      }//else
    }//if
    ok = true;
  }
  break;
case AcREMOVEPOINT:
  {
    //store the position of the removed point to be able to use it for selection of a next point
    //even if more than one point is selected, as long as we select the point befor the first selected point we are ok.
    //need to check if empty before select though!
    m_LastPosition = pointSet->SearchSelectedPoint();
    
    Operation* doOp = new mitk::Operation(OpREMOVEPOINT);
    if (m_UndoEnabled)
    {
      Operation* undoOp = new mitk::Operation(OpINSERTPOINT);
      OperationEvent *operationEvent = new OperationEvent(pointSet, doOp, undoOp, "Remove point");
      m_UndoController->SetOperationEvent(operationEvent);
    }
    pointSet->ExecuteOperation(doOp);
    ok = true;
  }
  break;
  case AcSELECT:
  //select the point found at the given mouseposition
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
    if (posEvent == NULL) 
      return false;

    mitk::Point3D worldPoint = posEvent->GetWorldPosition();

    //search the point in the list
    //distance set to 0, cause we already got the exact point from last State checkpoint
    //but we also need the position in the list to move it
    int position = pointSet->SearchPoint(worldPoint, 0);

    if (position>=0)//found a point
    {
      PointOperation* doOp = new mitk::PointOperation(OpSELECTPOINT, worldPoint, position);

      //Undo
      if (m_UndoEnabled)  //write to UndoMechanism
      {
        PointOperation* undoOp = new mitk::PointOperation(OpDESELECTPOINT, worldPoint, position);
        OperationEvent *operationEvent = new OperationEvent(pointSet, doOp, undoOp);
        m_UndoController->SetOperationEvent(operationEvent);
      }

      //execute the Operation
      pointSet->ExecuteOperation(doOp);
      ok = true;
    }
  }
  break;
  case AcDESELECT:
  {
    mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
    if (posEvent == NULL) 
      return false;

    mitk::Point3D worldPoint = posEvent->GetWorldPosition();

    //search the point in the list
    //distance set to 0, cause we already got the exact point from last State checkpoint
    //but we also need the position in the list to move it
    int position = pointSet->SearchPoint(worldPoint, 0);

    if (position>=0)//found a point
    {
      PointOperation* doOp = new mitk::PointOperation(OpDESELECTPOINT, worldPoint, position);

      //Undo
      if (m_UndoEnabled)  //write to UndoMechanism
      {
        PointOperation* undoOp = new mitk::PointOperation(OpSELECTPOINT, worldPoint, position);
        OperationEvent *operationEvent = new OperationEvent(pointSet, doOp, undoOp);
        m_UndoController->SetOperationEvent(operationEvent);
      }

      //execute the Operation
      pointSet->ExecuteOperation(doOp);
      ok = true;
      }
    }
    break;
  case AcSETSTARTPOINT:
    {
      //get the selected point and assign it as startpoint
      Point3D point;//dummy
      point.Fill(0);
      int position = pointSet->SearchSelectedPoint();
      if (position >-1)
      {
        PointOperation* doOp = new mitk::PointOperation(OpSETPOINTTYPE, point, position, true, PTSTART);

        //Undo
        if (m_UndoEnabled)  //write to UndoMechanism
        {
          PointOperation* undoOp = new mitk::PointOperation(OpSETPOINTTYPE, point, position, true, PTUNDEFINED);
          OperationEvent *operationEvent = new OperationEvent(pointSet, doOp, undoOp);
          m_UndoController->SetOperationEvent(operationEvent);
        }

        //execute the Operation
        pointSet->ExecuteOperation(doOp);
        ok = true;
      }
    }
    break;
    default:
    return Superclass::ExecuteAction( action, stateEvent );
  }

  return ok;

}

