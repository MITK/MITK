/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#include "mitkPointSetInteractor.h"
#include "mitkPointOperation.h"
#include "mitkPositionEvent.h"
#include "mitkPointSet.h"
#include "mitkStatusBar.h"
#include "mitkDataTreeNode.h"
#include "mitkInteractionConst.h"
#include "mitkAction.h"
#include "mitkStateEvent.h"
#include "mitkOperationEvent.h"
#include "mitkUndoController.h"
#include "mitkStateMachineFactory.h"
#include "mitkStateTransitionOperation.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"


//how precise must the user pick the point
//default value
const int PRECISION = 5;

mitk::PointSetInteractor
::PointSetInteractor(const char * type, DataTreeNode* dataTreeNode, int n)
:Interactor(type, dataTreeNode), m_N(n), m_Precision(PRECISION)
{
  m_LastPoint.Fill(0);
  m_SumVec.Fill(0);
}

mitk::PointSetInteractor::~PointSetInteractor()
{
}

void mitk::PointSetInteractor::SetPrecision(unsigned int precision)
{
  m_Precision = precision;
}

//##Documentation
//## overwritten cause this class can handle it better!
float mitk::PointSetInteractor
::CalculateJurisdiction(StateEvent const* stateEvent) const
{
  float returnValue = 0.0;
  //if it is a key event that can be handled in the current state, then return 0.5
  mitk::DisplayPositionEvent const  *disPosEvent =
    dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());

  //Key event handling:
  if (disPosEvent == NULL)
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

  //on MouseMove do nothing!
  if (stateEvent->GetEvent()->GetType() == mitk::Type_MouseMove)
  {
    return 0;
  }

  //if the event can be understood and if there is a transition waiting for that event
  if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
  {
    returnValue = 0.5;//it can be understood
  }

  int timeStep = disPosEvent->GetSender()->GetTimeStep();

  mitk::PointSet *pointSet = dynamic_cast<mitk::PointSet*>(m_DataTreeNode->GetData());
  if ( pointSet != NULL )
  {
    //if we have one point or more, then check if the have been picked
    if ( (pointSet->GetSize( timeStep ) > 0)
      && (pointSet->SearchPoint(
            disPosEvent->GetWorldPosition(), m_Precision, timeStep) > -1) )
    {
      returnValue = 1.0;
    }
  }
  return returnValue;
}

//TODO: add a new calculation of precision here! Input: StateEvent and Precision
//the method does a 2D picking with display coordinates and display geometry.
//Here the distance between the mouse position and the point is not as relative anymore!
//float mitk::PointSetInteractor::CalculatePrecision(float precision, mitk::StateEvent stateEvent)
//{
//  mitk::BaseRenderer *renderer = stateEvent->GetEvent()->GetSender();
//  if (renderer != NULL)
//  {
//    const mitk::DisplayGeometry* displayGeometry = renderer->GetDisplayGeometry();
//    if (displayGeometry != NULL)
//      displayGeometry->WorldToDisplay(, lineFrom);
//    precision =
//  }
//
//  return precision;
//
//}

void mitk::PointSetInteractor
::UnselectAll( int timeStep, ScalarType timeInMS )
{
  mitk::PointSet *pointSet = 
    dynamic_cast<mitk::PointSet*>( m_DataTreeNode->GetData() );
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
      mitk::PointOperation *doOp = new mitk::PointOperation(
        OpDESELECTPOINT, timeInMS, noPoint, position);
     
      if ( m_UndoEnabled )
      {
        mitk::PointOperation *undoOp = 
          new mitk::PointOperation(OpSELECTPOINT, timeInMS, noPoint, position);
        OperationEvent *operationEvent = 
          new OperationEvent( pointSet, doOp, undoOp );
        
        m_UndoController->SetOperationEvent( operationEvent );
      }

      pointSet->ExecuteOperation( doOp );
    }
  }
}

void mitk::PointSetInteractor
::SelectPoint( int position, int timeStep, ScalarType timeInMS )
{
  mitk::PointSet *pointSet = dynamic_cast< mitk::PointSet * >( 
    m_DataTreeNode->GetData() );

  //if List is empty, then no select of a point can be done!
  if ( (pointSet == NULL) || (pointSet->GetSize( timeStep ) <= 0) )
  {
    return;
  }

  //dummyPoint... not needed anyway
  mitk::Point3D noPoint;
  noPoint.Fill(0);

  mitk::PointOperation *doOp = new mitk::PointOperation(
    OpSELECTPOINT, timeInMS, noPoint, position);

  if ( m_UndoEnabled )
  {
    mitk::PointOperation* undoOp = new mitk::PointOperation(
      OpDESELECTPOINT, timeInMS, noPoint, position);

    OperationEvent *operationEvent = new OperationEvent(pointSet, doOp, undoOp);
    m_UndoController->SetOperationEvent(operationEvent);
  }

  pointSet->ExecuteOperation( doOp );
}


bool mitk::PointSetInteractor
::ExecuteAction( Action* action, mitk::StateEvent const* stateEvent )
{
  bool ok = false;//for return type bool

  //checking corresponding Data; has to be a PointSet or a subclass
  mitk::PointSet* pointSet = 
    dynamic_cast<mitk::PointSet*>(m_DataTreeNode->GetData());
  if ( pointSet == NULL )
  {
    return false;
  }

  const mitk::Event *event = stateEvent->GetEvent();

  int timeStep = 0;
  mitk::ScalarType timeInMS = 0.0;

  if ( event )
  {
    timeStep = event->GetSender()->GetTimeStep();
    timeInMS = event->GetSender()->GetTime();
  }

  // Make sure that the point set (if time-resolved) has enough entries;
  // if not, create the required extra ones (empty)
  pointSet->AdaptPointSetSeriesSize( timeStep+1 );

  //for reading on the points, Id's etc
  mitk::PointSet::DataType *itkPointSet = pointSet->GetPointSet( timeStep );
  if ( itkPointSet == NULL )
  {
    return false;
  }

  mitk::PointSet::PointsContainer *points = itkPointSet->GetPoints();

  /*Each case must watch the type of the event!*/
  switch (action->GetActionId())
  {
  case AcDONOTHING:
    ok = true;
    break;
  case AcCHECKOPERATION:
    //to check if the given Event is a DisplayPositionEvent.
    {
      mitk::DisplayPositionEvent const *dispPosEvent = 
        dynamic_cast <const mitk::DisplayPositionEvent *> (
          stateEvent->GetEvent());

      if (dispPosEvent != NULL)
      {
        mitk::StateEvent* newStateEvent = 
          new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
        this->HandleEvent( newStateEvent );
      }
      else
      {
        mitk::StateEvent* newStateEvent = 
          new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
        this->HandleEvent( newStateEvent );
      }
      ok = true;
      break;
    }

  case AcADDPOINT:
    // Declare two operations: one for the selected state: deselect the last 
    // one selected and select the new one the other operation is the add 
    // operation: There the first empty place have to be found and the new
    // point inserted into that space
    {
      mitk::DisplayPositionEvent const *posEvent = 
        dynamic_cast < const mitk::DisplayPositionEvent * > 
          (stateEvent->GetEvent());

      // Check if it is a DisplayEvent thrown in a 3D window. Then the
      // z-information is missing. Returning false might end in the state
      // full, but the last point couldn't be added, so the set wouldn't be
      // full. So a extra Action that checks the operationtype has been added.
      if ( posEvent == NULL )
      {
        return false;
      }

      mitk::Point3D itkPoint;
      itkPoint = posEvent->GetWorldPosition();

      // undo-supported deselect of all points in the DataList; if List is
      // empty, then nothing will be unselected
      this->UnselectAll( timeStep, timeInMS );

      // find the position, the point is to be added to: first entry with
      // empty index. If the Set is empty, then start with 0. if not empty, 
      // then take the first index not occupied
      int lastPosition = 0;
      if (!points->empty())
      {
        mitk::PointSet::PointsIterator it, end;
        it = points->Begin();
        end = points->End();
        while( it != end )
        {
          if (!points->IndexExists(lastPosition))
            break;
          ++it;
          ++lastPosition;
        }
      }

      PointOperation* doOp = new mitk::PointOperation(
        OpINSERT, timeInMS, itkPoint, lastPosition);

      if (m_UndoEnabled)
      {
        // difference between OpDELETE and OpREMOVE is, that OpDELETE deletes 
        // a point at the end, and OpREMOVE deletes it from the given position
        // remove is better, cause we need the position to add or remove the 
        // point anyway. We can get the last position from size()
        PointOperation *undoOp = new mitk::PointOperation(
          OpREMOVE, timeInMS, itkPoint, lastPosition);
        OperationEvent *operationEvent = 
          new OperationEvent(pointSet, doOp, undoOp, "Add point");
        m_UndoController->SetOperationEvent(operationEvent);
      }

      //execute the Operation
      pointSet->ExecuteOperation(doOp);

      //outquoted, cause change in PointSet: the point is added and directly
      //selected.
      ////now select the new point
      ////lastposition was the size, then the list got incremended, now 
      ////lastposition is the index of the last element in list
      //PointOperation* doSelOp = new mitk::PointOperation(OpSELECTPOINT, 
      //  timeInMS, itkPoint, lastPosition);
      //if (m_UndoEnabled)
      //{
      //  PointOperation* undoSelOp = new mitk::PointOperation(OpDESELECTPOINT, 
      //    timeInMS, itkPoint, lastPosition);
      //  OperationEvent *selOperationEvent = new OperationEvent(pointSet, 
      //    doSelOp, undoSelOp);
      //  m_UndoController->SetOperationEvent(selOperationEvent);
      //}
      //pointSet->ExecuteOperation(doSelOp);

      ok = true;

      // Update the display
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      break;
    }
  case AcINITMOVEMENT:
    {
      mitk::PositionEvent const *posEvent = 
        dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
      
      if (posEvent == NULL)
        return false;

      // start of the Movement is stored to calculate the undoKoordinate
      // in FinishMovement
      m_LastPoint = posEvent->GetWorldPosition();

      // initialize a value to calculate the movement through all
      // MouseMoveEvents from MouseClick to MouseRelease
      m_SumVec.Fill(0);

      ok = true;
      break;
    }
  case AcMOVESELECTED://moves all selected Elements
    {
      mitk::PositionEvent const *posEvent = 
        dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());

      if (posEvent == NULL)
        return false;

      mitk::Point3D newPoint, resultPoint;
      newPoint = posEvent->GetWorldPosition();
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
      it = points->Begin();
      end = points->End();
      while( it != end )
      {
        int position = it->Index();
        if ( pointSet->GetSelectInfo(position, timeStep) )//if selected
        {
          PointSet::PointType pt = pointSet->GetPoint(position, timeStep);
          mitk::Point3D sumVec;
          sumVec[0] = pt[0];
          sumVec[1] = pt[1];
          sumVec[2] = pt[2];
          resultPoint = sumVec + dirVector;
          PointOperation* doOp = new mitk::PointOperation(OpMOVE, timeInMS, 
            resultPoint, position);

          //execute the Operation
          //here no undo is stored, because the movement-steps aren't interesting. 
          // only the start and the end is interisting to store for undo.
          pointSet->ExecuteOperation(doOp);
        }
        ++it;
      }
      m_LastPoint = newPoint;//for calculation of the direction vector
      ok = true;

      // Update the display
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      break;
    }
  
  case AcREMOVEPOINT://remove the given Point from the list
    {
      //if the point to be removed is given by the positionEvent:
      mitk::PositionEvent const  *posEvent = 
        dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
      if (posEvent != NULL)
      {
        mitk::Point3D itkPoint;
        itkPoint = posEvent->GetWorldPosition();

        //search the point in the list
        int position = pointSet->SearchPoint(itkPoint, 0.0, timeStep);
        //distance set to 0, cause we already got the exact point from last
        //State checkpointbut we also need the position in the list to remove it
        if (position>=0)//found a point
        {
          PointSet::PointType pt = pointSet->GetPoint(position, timeStep);
          itkPoint[0] = pt[0];
          itkPoint[1] = pt[1];
          itkPoint[2] = pt[2];

          //Undo
          PointOperation* doOp = new mitk::PointOperation(OpREMOVE,
            timeInMS, itkPoint, position);
          if (m_UndoEnabled)  //write to UndoMechanism
          {
            PointOperation* undoOp = new mitk::PointOperation(OpINSERT, 
              timeInMS, itkPoint, position);
            OperationEvent *operationEvent = new OperationEvent(pointSet, 
              doOp, undoOp, "Remove point");
            m_UndoController->SetOperationEvent(operationEvent);
          }
          //execute the Operation
          pointSet->ExecuteOperation(doOp);

          /*now select the point "position-1",
          and if it is the first in list,
          then contine at the last in list*/
          
          
          //only then a select of a point is possible!
          if (pointSet->GetSize( timeStep ) > 0)
          {
            if (position>0)//not the first in list
            {
              this->SelectPoint( position-1, timeStep, timeInMS );
            }
            //it was the first point in list, that was removed, so select 
            //the last in list
            else
            {
              position = pointSet->GetSize( timeStep ) - 1; //last in list
              this->SelectPoint( position, timeStep, timeInMS );
            }//else
          }//if

          ok = true;
        }
      }
      else //no position is given so remove all selected elements
      {
        //delete all selected points
        //search for the selected one and then declare the operations!
        mitk::PointSet::PointsContainer::Iterator it, end;
        it = points->Begin();
        end = points->End();
        int position = 0;
        while (it != end)
        {
          //if point is selected
          if (  pointSet->GetSelectInfo(it->Index(), timeStep) )
          {
            //get the coordinates of that point to be undoable
            PointSet::PointType selectedPoint = it->Value();
            mitk::Point3D itkPoint;
            itkPoint[0] = selectedPoint[0];
            itkPoint[1] = selectedPoint[1];
            itkPoint[2] = selectedPoint[2];

            position = it->Index();
            PointOperation* doOp = new mitk::PointOperation(OpREMOVE, 
              timeInMS, itkPoint, position);
            //Undo
            if (m_UndoEnabled)  //write to UndoMechanism
            {
              PointOperation* undoOp = new mitk::PointOperation(OpINSERT,
                timeInMS, itkPoint, position);
              OperationEvent *operationEvent = new OperationEvent(pointSet, 
                doOp, undoOp, "Remove point");
              m_UndoController->SetOperationEvent(operationEvent);
            }
            pointSet->ExecuteOperation(doOp);

            //after delete the iterator is undefined, so start again and 
            //count to the entry before! If there ist only one point, then 
            //rather start at the begining, cause that is the point then.
            it = points->Begin();
            if (points->Size()>1)
            {
              for (int counter = 0; counter < position-1; counter++)
                it++;
            }

            if (it == end) { break; }

          }//if
          it++;
        }//while
        /*now select the point before the point/points that was/were deleted*/
        
        //only then a select of a point is possible!
        if (pointSet->GetSize( timeStep ) > 0)
        {
          //to not loose the position of the deleted point
          int aPosition = position;
          bool found = false;

          --aPosition;//begin before the one deleted
          for (;aPosition >= 0; --aPosition)//search backwards
          {
            if (points->IndexExists(aPosition))
            {
              found = true;
              break;
            }
          }

          if (!found)//no element before position! so look afterwards!
          {
            mitk::PointSet::PointsContainer::Iterator it, end;
            it = points->Begin();
            end = points->End();
            //go to the position in front of the point, that was deleted
            for (int counter = 0; counter < position-1; ++counter)
            {
              ++it;
            }
            aPosition = it->Index();
          }
          this->SelectPoint( aPosition, timeStep, timeInMS );
        }//if
        ok = true;
      }//else
    }

    // Update the display
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    break;

  // Remove all Points that have been set at once. 
  // TODO: Undo function not supported yet.
  case AcREMOVEALL:
    {
      if ( !points->empty() )
      {
        PointSet::PointType pt;
        mitk::PointSet::PointsContainer::Iterator it, end;
        it = points->Begin();
        end = points->End();
        int position = 0;
        while ( it != end )
        {
          position = it->Index();
          if ( points->IndexExists( position ) )
          {
            pt = pointSet->GetPoint( position, timeStep );
            PointOperation* doOp = 
              new mitk::PointOperation( OpREMOVE, timeInMS, pt, position );
            ++it;
            pointSet->ExecuteOperation( doOp );
          }
          else it++;
        }
      }

      // Update the display
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      break;
    }

  //Checking if the Point transmitted is close enough to one point. Then 
  //generate a new event with the point and let this statemaschine
  //handle the event.
  case AcCHECKELEMENT:
    {
      mitk::PositionEvent const  *posEvent = 
        dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
      if (posEvent != NULL)
      {
        mitk::Point3D worldPoint = posEvent->GetWorldPosition();

        int position = pointSet->SearchPoint( worldPoint, m_Precision, timeStep );
        if (position>=0)//found a point near enough to the given point
        {
          //get that point, the one meant by the user!
          PointSet::PointType pt = pointSet->GetPoint(position, timeStep);
          mitk::Point2D displPoint;
          displPoint[0] = worldPoint[0]; displPoint[1] = worldPoint[1];
          //new Event with information YES and with the correct point
          mitk::PositionEvent const* newPosEvent = 
            new mitk::PositionEvent(posEvent->GetSender(), Type_None, 
            BS_NoButton, BS_NoButton, Key_none, displPoint, pt);
          mitk::StateEvent* newStateEvent =
            new mitk::StateEvent(EIDYES, newPosEvent);
          //call HandleEvent to leave the guard-state
          this->HandleEvent( newStateEvent );
          ok = true;
        }
        else
        {
          //new Event with information NO
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, posEvent);
          this->HandleEvent(newStateEvent );
          ok = true;
        }
      }
      else
      {
        mitk::DisplayPositionEvent const  *disPosEvent = 
          dynamic_cast <const mitk::DisplayPositionEvent *> (
            stateEvent->GetEvent());
        if (disPosEvent != NULL)
        { //2d Koordinates for 3D Interaction; return false to redo
          //the last statechange
          mitk::StateEvent* newStateEvent = 
            new mitk::StateEvent(EIDNO, posEvent);
          this->HandleEvent(newStateEvent );
          ok = true;
        }
      }

      break;
    }

  case AcCHECKSELECTED:
    /*check, if the given point is selected:
    if no, then send EIDNO
    if yes, then send EIDYES*/

    // check, if: because of the need to look up the point again, it is 
    // possible, that we grab the wrong point in case there are two same points
    // so maybe we do have to set a global index for further computation,
    // as long, as the mouse is moved...
    {
      int position = -1;
      mitk::PositionEvent const  *posEvent = 
        dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
      if (posEvent == NULL)
        return false;
      mitk::Point3D worldPoint = posEvent->GetWorldPosition();

      position = pointSet->SearchPoint(worldPoint, m_Precision, timeStep);

      if (position>=0)
      {
        mitk::PositionEvent const  *newPosEvent = 
          new mitk::PositionEvent(posEvent->GetSender(),
          posEvent->GetType(), posEvent->GetButton(), 
          posEvent->GetButtonState(), posEvent->GetKey(),
          posEvent->GetDisplayPosition(), posEvent->GetWorldPosition());

        //if selected on true, then call Event EIDYES
        if (pointSet->GetSelectInfo(position, timeStep))
        {
          mitk::StateEvent* newStateEvent = 
            new mitk::StateEvent( EIDYES, newPosEvent );
          this->HandleEvent( newStateEvent );
          ok = true;

          //saving the spot for calculating the direction vector in moving
          m_LastPoint = posEvent->GetWorldPosition();
        }
        else //not selected then call event EIDNO
        {
          //new Event with information NO
          mitk::StateEvent* newStateEvent = 
            new mitk::StateEvent( EIDNO, newPosEvent );
          this->HandleEvent( newStateEvent );
          ok = true;
        }
      }
      //the position wasn't set properly. If necessary: search the given 
      //point in list and set var position
      else
      {
        mitk::StatusBar::GetInstance()->DisplayText(
          "Message from mitkPointSetInteractor: Error in Actions! Check Config XML-file", 
          10000);
        ok = false;
      }
  
      break;
    }
  
  //generate Events if the set will be full after the addition of the 
  // point or not.
  case AcCHECKNMINUS1:
    {
      // number of points not limited->pass on
      // "Amount of points in Set is smaller then N-1"
      if (m_N<0)
      {
        mitk::StateEvent* newStateEvent = 
          new mitk::StateEvent(EIDSTSMALERNMINUS1, stateEvent->GetEvent());
        this->HandleEvent( newStateEvent );
        ok = true;
      }
      else
      {
        if (pointSet->GetSize( timeStep ) < m_N-1 )
          //pointset after addition won't be full
        {
          mitk::StateEvent* newStateEvent = 
            new mitk::StateEvent(EIDSTSMALERNMINUS1, stateEvent->GetEvent());
          this->HandleEvent( newStateEvent );
          ok = true;
        }
        else
          //after the addition of a point, the container will be full
        {
          mitk::StateEvent* newStateEvent = 
            new mitk::StateEvent(EIDSTLARGERNMINUS1, stateEvent->GetEvent());
          this->HandleEvent(newStateEvent );
          ok = true;
        }//else
      }//else
    }
    break;

  case AcCHECKEQUALS1:
    {
      //the number of points in the list is 1 (or smaler)
      if (pointSet->GetSize( timeStep ) <= 1)
      {
        mitk::StateEvent* newStateEvent = 
          new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
        this->HandleEvent( newStateEvent );
        ok = true;
      }
      else //more than 1 points in list, so stay in the state!
      {
        mitk::StateEvent* newStateEvent = 
          new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
        this->HandleEvent(newStateEvent );
        ok = true;
      }
    }
    break;

  case AcSELECTPICKEDOBJECT://and deselect others
    {
      mitk::PositionEvent const  *posEvent = 
        dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
      if (posEvent == NULL) return false;

      mitk::Point3D itkPoint;
      itkPoint = posEvent->GetWorldPosition();

      //search the point in the list
      int position = pointSet->SearchPoint(itkPoint, 0.0, timeStep);
      //distance set to 0, cause we already got the exact point from last
      //State checkpoint but we also need the position in the list to move it
      if (position>=0)//found a point
      {
        //first deselect the other points
        //undoable deselect of all points in the DataList
        this->UnselectAll( timeStep, timeInMS);

        PointOperation* doOp = new mitk::PointOperation(OpSELECTPOINT,
          timeInMS, itkPoint, position);

        //Undo
        if (m_UndoEnabled)  //write to UndoMechanism
        {
          PointOperation* undoOp = new mitk::PointOperation(OpDESELECTPOINT, 
            timeInMS, itkPoint, position);
          OperationEvent *operationEvent = 
            new OperationEvent(pointSet, doOp, undoOp);
          m_UndoController->SetOperationEvent(operationEvent);
        }

        //execute the Operation
        pointSet->ExecuteOperation(doOp);
        ok = true;
      }

      // Update the display
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      break;
    }

  case AcDESELECTOBJECT:
    {
      mitk::PositionEvent const  *posEvent = 
        dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
      if (posEvent == NULL)
        return false;

      mitk::Point3D itkPoint;
      itkPoint = posEvent->GetWorldPosition();

      //search the point in the list
      int position = pointSet->SearchPoint(itkPoint, 0.0, timeStep);

      //distance set to 0, cause we already got the exact point from last 
      // State checkpoint but we also need the position in the list to move it
      if (position>=0)//found a point
      {
        //Undo
        PointOperation* doOp = new mitk::PointOperation(OpDESELECTPOINT,
          timeInMS, itkPoint, position);
        if (m_UndoEnabled)  //write to UndoMechanism
        {
          PointOperation* undoOp = new mitk::PointOperation(OpSELECTPOINT,
            timeInMS, itkPoint, position);
          OperationEvent *operationEvent = new OperationEvent(pointSet, doOp, undoOp);
          m_UndoController->SetOperationEvent(operationEvent);
        }
        //execute the Operation
        pointSet->ExecuteOperation(doOp);
        ok = true;
      }

      // Update the display
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      break;
    }

  case AcDESELECTALL:
    {
      //undo-supported able deselect of all points in the DataList
      this->UnselectAll( timeStep, timeInMS );
      ok = true;

      // Update the display
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      break;
    }

  case AcFINISHMOVEMENT:
    {
      mitk::PositionEvent const *posEvent = 
        dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
      if (posEvent == NULL)
        return false;

      //finish the movement:
      //the final point is m_LastPoint
      //m_SumVec stores the movement in a vector
      //the operation would not be necessary, but we need it for the undo Operation.
      //m_LastPoint is for the Operation
      //the point for undoOperation calculates from all selected 
      //elements (point) - m_SumVec

      //search all selected elements and move them with undo-functionality.
      mitk::PointSet::PointsIterator it, end;
      it = points->Begin();
      end = points->End();
      while( it != end )
      {
        int position = it->Index();
        if ( pointSet->GetSelectInfo(position, timeStep) )//if selected
        {
          PointSet::PointType pt = pointSet->GetPoint(position, timeStep);
          Point3D itkPoint;
          itkPoint[0] = pt[0];
          itkPoint[1] = pt[1];
          itkPoint[2] = pt[2];
          PointOperation* doOp = new mitk::PointOperation(OpMOVE,
            timeInMS, itkPoint, position);

          if ( m_UndoEnabled )//&& (posEvent->GetType() == mitk::Type_MouseButtonRelease)
          {
            //set the undo-operation, so the final position is undo-able
            //calculate the old Position from the already moved position - m_SumVec
            mitk::Point3D undoPoint = ( itkPoint - m_SumVec );
            PointOperation* undoOp = 
              new mitk::PointOperation(OpMOVE, timeInMS, undoPoint, position);
            OperationEvent *operationEvent = 
              new OperationEvent(pointSet, doOp, undoOp, "Move point");
            m_UndoController->SetOperationEvent(operationEvent);
          }
          //execute the Operation
          pointSet->ExecuteOperation(doOp);
        }
        ++it;
      }

      //set every variable for movement calculation to zero
      // commented out: increases usebility in derived classes.
      /*m_LastPoint.Fill(0);
      m_SumVec.Fill(0);*/

      //increase the GroupEventId, so that the Undo goes to here
      this->IncCurrGroupEventId();
      ok = true;

      // Update the display
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      break;
    }
  
  case AcCLEAR:
    {
      this->Clear( timeStep, timeInMS );

      // Update the display
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      break;
    }

  default:
    return Superclass::ExecuteAction( action, stateEvent );
  }

  return ok;
}

void mitk::PointSetInteractor::Clear( int timeStep, ScalarType timeInMS )
{
  mitk::Point3D point;
  point.Fill(0);

  mitk::PointSet *pointSet = 
    dynamic_cast<mitk::PointSet*>(m_DataTreeNode->GetData());
  if ( pointSet == NULL )
  {
    return;
  }

  mitk::PointSet::DataType *itkPointSet = pointSet->GetPointSet( timeStep );
  if ( itkPointSet == NULL )
  {
    return;
  }

  //for reading on the points, Id's etc
  mitk::PointSet::PointsContainer *points = itkPointSet->GetPoints();

  mitk::PointSet::PointsIterator it, end;
  it = points->Begin();
  end = points->End();
  while( (it != end) && (pointSet->GetSize( timeStep ) > 0) )
  {
    point = pointSet->GetPoint( it->Index(), timeStep );
    PointOperation *doOp = new mitk::PointOperation(
      OpREMOVE, timeInMS, point, it->Index());

    //write to UndoMechanism
    if ( m_UndoEnabled )
    {
      PointOperation *undoOp = new mitk::PointOperation(
        OpINSERT, timeInMS, point, it->Index());

      OperationEvent *operationEvent = 
        new OperationEvent( pointSet, doOp, undoOp );

      m_UndoController->SetOperationEvent( operationEvent );
    }
    //execute the Operation
    
    ++it;
    pointSet->ExecuteOperation( doOp );
  }

  //reset the statemachine
  this->ResetStatemachineToStartState();
}

