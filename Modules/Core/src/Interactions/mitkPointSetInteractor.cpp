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


#include "mitkPointSetInteractor.h"
#include "mitkPointOperation.h"
#include "mitkPositionEvent.h"
#include "mitkPointSet.h"
//#include "mitkStatusBar.h"
#include "mitkDataNode.h"
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
::PointSetInteractor(const char * type, DataNode* dataNode, int n)
:Interactor(type, dataNode), m_Precision(PRECISION), m_N(n)
{
  if (m_N==0)
  {
    STATEMACHINE_WARN<<"Instanciation of PointSetInteractor which takes care of 0 points does't make sense!\n";
    STATEMACHINE_WARN<<"Setting number of points to 1!\n";
    m_N = 1;
  }

  m_LastPoint.Fill(0);
  m_SumVec.Fill(0);
  this->InitAccordingToNumberOfPoints();
}

mitk::PointSetInteractor::~PointSetInteractor()
{
}

//##Documentation
//## overwritten cause this class can handle it better!
float mitk::PointSetInteractor::CanHandleEvent(StateEvent const* stateEvent) const
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
    {return 0.5;}
    else
    {return 0;}
  }
  //on MouseMove do nothing!
  if (stateEvent->GetEvent()->GetType() == mitk::Type_MouseMove)
  {return 0;}
  //get the time of the sender to look for the right transition.
  mitk::BaseRenderer* sender = stateEvent->GetEvent()->GetSender();
  if (sender != NULL)
  {
    int timeStep = sender->GetTimeStep(m_DataNode->GetData());
    //if the event can be understood and if there is a transition waiting for that event
    mitk::State const* state = this->GetCurrentState(timeStep);
    if (state!= NULL)
      if (state->GetTransition(stateEvent->GetId())!=NULL)
        returnValue = 0.5;//it can be understood
    mitk::PointSet *pointSet = dynamic_cast<mitk::PointSet*>(m_DataNode->GetData());
    if ( pointSet != NULL )
    {
      //if we have one point or more, then check if the have been picked
      if ( (pointSet->GetSize( timeStep ) > 0)
        && (pointSet->SearchPoint(
        disPosEvent->GetWorldPosition(), (ScalarType) m_Precision, timeStep) > -1) )
      {returnValue = 1.0;}
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

void mitk::PointSetInteractor::UnselectAll( unsigned int timeStep, ScalarType timeInMS )
{
  mitk::PointSet *pointSet =
    dynamic_cast<mitk::PointSet*>( m_DataNode->GetData() );
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

      if ( !m_UndoEnabled )
        delete doOp;
    }
  }
}

void mitk::PointSetInteractor::SelectPoint( int position, unsigned int timeStep, ScalarType timeInMS )
{
  mitk::PointSet *pointSet = dynamic_cast< mitk::PointSet * >(
    m_DataNode->GetData() );

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

  if ( !m_UndoEnabled )
    delete doOp;

}


bool mitk::PointSetInteractor::ExecuteAction( Action* action, mitk::StateEvent const* stateEvent )
{
  bool ok = false;//for return type bool

  //checking corresponding Data; has to be a PointSet or a subclass
  mitk::PointSet* pointSet =
    dynamic_cast<mitk::PointSet*>(m_DataNode->GetData());
  if ( pointSet == NULL )
  {
    return false;
  }

  //get the timestep to support 3D+T
  const mitk::Event *theEvent = stateEvent->GetEvent();
  mitk::ScalarType timeInMS = 0.0;

  //check if the current timestep has to be changed
  if ( theEvent )
  {
    if (theEvent->GetSender() != NULL)
    {
      //additionaly to m_TimeStep we need timeInMS to satisfy the execution of the operations
      timeInMS = theEvent->GetSender()->GetTime();
    }
  }

  //for reading on the points, Id's etc
  mitk::PointSet::DataType *itkPointSet = pointSet->GetPointSet( m_TimeStep );
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
        mitk::StateEvent newStateEvent(EIDYES, stateEvent->GetEvent());
        this->HandleEvent( &newStateEvent );
      }
      else
      {
        mitk::StateEvent newStateEvent(EIDNO, stateEvent->GetEvent());
        this->HandleEvent( &newStateEvent );
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
      this->UnselectAll( m_TimeStep, timeInMS );

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

      if ( !m_UndoEnabled )
        delete doOp;

      //the point is added and directly selected in PintSet. So no need to call OpSELECTPOINT

      ok = true;

      // Update the display
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      break;
    }
  case AcINITMOVEMENT:
    {
      mitk::PositionEvent const *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());

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
      mitk::PositionEvent const *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());

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
        if ( pointSet->GetSelectInfo(position, m_TimeStep) )//if selected
        {
          PointSet::PointType pt = pointSet->GetPoint(position, m_TimeStep);
          mitk::Point3D sumVec;
          sumVec[0] = pt[0];
          sumVec[1] = pt[1];
          sumVec[2] = pt[2];
          resultPoint = sumVec + dirVector;
          PointOperation doOp(OpMOVE, timeInMS, resultPoint, position);

          //execute the Operation
          //here no undo is stored, because the movement-steps aren't interesting.
          // only the start and the end is interisting to store for undo.
          pointSet->ExecuteOperation(&doOp);
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
        int position = pointSet->SearchPoint(itkPoint, 0.0, m_TimeStep);
        //distance set to 0, cause we already got the exact point from last
        //State checkpointbut we also need the position in the list to remove it
        if (position>=0)//found a point
        {
          PointSet::PointType pt = pointSet->GetPoint(position, m_TimeStep);
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

          if ( !m_UndoEnabled )
            delete doOp;

          //only then a select of a point is possible!
          if (pointSet->GetSize( m_TimeStep ) > 0)
          {
            this->SelectPoint(pointSet->Begin(m_TimeStep)->Index(), m_TimeStep, timeInMS);
          }

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
        int previousExistingPosition = -1;//to recognize the last existing position; needed because the iterator gets invalid if the point is deleted!
        int lastDelPrevExistPosition = -1; //the previous position of the last deleted point
        while (it != end)
        {
          if (points->IndexExists(it->Index()))
          {
            //if point is selected
            if (  pointSet->GetSelectInfo(it->Index(), m_TimeStep) )
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

              if ( !m_UndoEnabled )
                delete doOp;

              //after delete the iterator is undefined, so start again
              //count to the last existing entry
              if (points->Size()>1 && points->IndexExists(previousExistingPosition))
              {
                for (it = points->Begin(); it != points->End(); it++)
                {
                  if (it->Index() == (unsigned int) previousExistingPosition)
                  {
                    lastDelPrevExistPosition = previousExistingPosition;
                    break; //return if the iterator on the last existing position is found
                  }
                }
              }
              else // size <= 1 or no previous existing position set
              {
                //search for the first existing position
                for (it = points->Begin(); it != points->End(); it++)
                  if (points->IndexExists(it->Index()))
                  {
                    previousExistingPosition = it->Index();
                    break;
                  }
              }

              //now that we have set the iterator, lets get sure, that the next it++ will not crash!
              if (it == end) { break; }

            }//if
            else
            {
              previousExistingPosition = it->Index();
            }
          }//if index exists

          it++;
        }//while

        if (lastDelPrevExistPosition < 0)//the var has not been set because the first element was deleted and there was no prev position
          lastDelPrevExistPosition = previousExistingPosition; //go to the end

        /*
        * now select the point before the point/points that was/were deleted
        */
        if (pointSet->GetSize( m_TimeStep ) > 0) //only then a select of a point is possible!
        {
          if (points->IndexExists(lastDelPrevExistPosition))
          {
            this->SelectPoint( lastDelPrevExistPosition, m_TimeStep, timeInMS );
          }
          else
          {
            //select the first existing element
            for (mitk::PointSet::PointsContainer::Iterator it = points->Begin(); it != points->End(); it++)
              if (points->IndexExists(it->Index()))
              {
                this->SelectPoint( it->Index(), m_TimeStep, timeInMS );
                break;
              }
          }
        }//if
        ok = true;
      }//else
    }//case

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
            pt = pointSet->GetPoint( position, m_TimeStep );
            PointOperation doOp( OpREMOVE, timeInMS, pt, position );
            ++it;
            pointSet->ExecuteOperation( &doOp );
          }
          else it++;
        }
      }
      ok = true;
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

        int position = pointSet->SearchPoint( worldPoint, m_Precision, m_TimeStep );
        if (position>=0)//found a point near enough to the given point
        {
          //get that point, the one meant by the user!
          PointSet::PointType pt = pointSet->GetPoint(position, m_TimeStep);
          mitk::Point2D displPoint;
          displPoint[0] = worldPoint[0]; displPoint[1] = worldPoint[1];
          //new Event with information YES and with the correct point
          mitk::PositionEvent newPosEvent(posEvent->GetSender(), Type_None,
            BS_NoButton, BS_NoButton, Key_none, displPoint, pt);
          mitk::StateEvent newStateEvent(EIDYES, &newPosEvent);
          //call HandleEvent to leave the guard-state
          this->HandleEvent( &newStateEvent );
          ok = true;
        }
        else
        {
          //new Event with information NO
          mitk::StateEvent newStateEvent(EIDNO, posEvent);
          this->HandleEvent(&newStateEvent );
          ok = true;
        }
      }
      else
      {
        MITK_DEBUG("OperationError")<<this->GetType()<<" AcCHECKELEMENT expected PointOperation.";

        mitk::DisplayPositionEvent const  *disPosEvent =
          dynamic_cast <const mitk::DisplayPositionEvent *> (
            stateEvent->GetEvent());
        if (disPosEvent != NULL)
        { //2d Koordinates for 3D Interaction; return false to redo
          //the last statechange
          mitk::StateEvent newStateEvent(EIDNO, disPosEvent);
          this->HandleEvent(&newStateEvent);
          ok = true;
        }
      }

      break;
    }
  case AcCHECKONESELECTED:
    //check if there is a point that is selected
    {
      if (pointSet->GetNumberOfSelected(m_TimeStep)>0)
      {
        mitk::StateEvent newStateEvent( EIDYES, theEvent);
        this->HandleEvent( &newStateEvent );
      }
      else //not selected then call event EIDNO
      {
        //new Event with information NO
        mitk::StateEvent newStateEvent( EIDNO, theEvent);
        this->HandleEvent( &newStateEvent );
      }
      ok = true;
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

      position = pointSet->SearchPoint(worldPoint, m_Precision, m_TimeStep);

      if (position>=0)
      {
        mitk::PositionEvent const  *newPosEvent =
          new mitk::PositionEvent(posEvent->GetSender(),
          posEvent->GetType(), posEvent->GetButton(),
          posEvent->GetButtonState(), posEvent->GetKey(),
          posEvent->GetDisplayPosition(), posEvent->GetWorldPosition());

        //if selected on true, then call Event EIDYES
        if (pointSet->GetSelectInfo(position, m_TimeStep))
        {
          mitk::StateEvent newStateEvent( EIDYES, newPosEvent );
          this->HandleEvent( &newStateEvent );
          ok = true;

          //saving the spot for calculating the direction vector in moving
          m_LastPoint = posEvent->GetWorldPosition();
        }
        else //not selected then call event EIDNO
        {
          //new Event with information NO
          mitk::StateEvent newStateEvent( EIDNO, newPosEvent );
          this->HandleEvent( &newStateEvent );
          ok = true;
        }
        delete newPosEvent;
      }
      //the position wasn't set properly. If necessary: search the given
      //point in list and set var position
      else
      {
        /*
         mitk::StatusBar::GetInstance()->DisplayText(
          "Message from mitkPointSetInteractor: Error in Actions! Check Config XML-file",
          10000);
        */
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
        mitk::StateEvent newStateEvent(EIDSTSMALERNMINUS1, stateEvent->GetEvent());
        this->HandleEvent( &newStateEvent );
        ok = true;
      }
      else
      {
        if (pointSet->GetSize( m_TimeStep ) < m_N-1 )
          //pointset after addition won't be full
        {
          mitk::StateEvent newStateEvent(EIDSTSMALERNMINUS1, stateEvent->GetEvent());
          this->HandleEvent( &newStateEvent );
          ok = true;
        }
        else
          //after the addition of a point, the container will be full
        {
          mitk::StateEvent newStateEvent(EIDSTLARGERNMINUS1, stateEvent->GetEvent());
          this->HandleEvent( &newStateEvent );
          ok = true;
        }//else
      }//else
    }
    break;
  case AcCHECKEQUALS1:
    {
      //the number of points in the list is 1 (or smaler)
      if (pointSet->GetSize( m_TimeStep ) <= 1)
      {
        mitk::StateEvent newStateEvent(EIDYES, stateEvent->GetEvent());
        this->HandleEvent( &newStateEvent );
        ok = true;
      }
      else //more than 1 points in list, so stay in the state!
      {
        mitk::StateEvent newStateEvent(EIDNO, stateEvent->GetEvent());
        this->HandleEvent( &newStateEvent );
        ok = true;
      }
    }
    break;
  case AcCHECKNUMBEROFPOINTS:
    {
      //the number of points in the list is 1 (or smaler), so will be empty after delete
     if (pointSet->GetSize( m_TimeStep ) <= 1)
      {
        mitk::StateEvent newStateEvent(EIDEMPTY, stateEvent->GetEvent());
        this->HandleEvent( &newStateEvent );
        ok = true;
      }
      else if (pointSet->GetSize( m_TimeStep ) <= m_N || m_N <= -1)
       //m_N is set to unlimited points allowed or more than 1 points in list, but not full, so stay in the state!
     {
       // if the number of points equals m_N and no point of the point set is selected switch to state EIDEQUALSN
       if ((pointSet->GetSize( m_TimeStep ) == m_N)&&(pointSet->GetNumberOfSelected()==0))
       {
         mitk::StateEvent newStateEvent(EIDEQUALSN, stateEvent->GetEvent());
         this->HandleEvent( &newStateEvent );
         ok = true;
       }
       // if the number of points is small than or equal m_N and point(s) are selected stay in state
       else
       {
         mitk::StateEvent newStateEvent(EIDSMALLERN, stateEvent->GetEvent());
         this->HandleEvent( &newStateEvent );
         ok = true;
       }
     }
      else
        //pointSet->GetSize( m_TimeStep ) >=m_N.
        // This can happen if the points were not added
        // by interaction but by loading a .mps file
      {
        mitk::StateEvent newStateEvent(EIDEQUALSN, stateEvent->GetEvent());
        this->HandleEvent( &newStateEvent );
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
      int position = pointSet->SearchPoint(itkPoint, 0.0, m_TimeStep);
      //distance set to 0, cause we already got the exact point from last
      //State checkpoint but we also need the position in the list to move it
      if (position>=0)//found a point
      {
        //first deselect the other points
        //undoable deselect of all points in the DataList
        this->UnselectAll( m_TimeStep, timeInMS);

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

        if ( !m_UndoEnabled )
          delete doOp;

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
      int position = pointSet->SearchPoint(itkPoint, 0.0, m_TimeStep);

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

        if ( !m_UndoEnabled )
          delete doOp;

        ok = true;
      }

      // Update the display
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      break;
    }

  case AcDESELECTALL:
    {
      //undo-supported able deselect of all points in the DataList
      this->UnselectAll( m_TimeStep, timeInMS );
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
        if ( pointSet->GetSelectInfo(position, m_TimeStep) )//if selected
        {
          PointSet::PointType pt = pointSet->GetPoint(position, m_TimeStep);
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

          if ( !m_UndoEnabled )
            delete doOp;

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
      this->Clear( m_TimeStep, timeInMS );

      // Update the display
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      break;
    }

  default:
    return Superclass::ExecuteAction( action, stateEvent );
  }
  // indicate modification of data tree node
  m_DataNode->Modified();
  return ok;
}

void mitk::PointSetInteractor::Clear( unsigned int timeStep, ScalarType timeInMS )
{
  mitk::Point3D point;
  point.Fill(0);

  mitk::PointSet *pointSet =
    dynamic_cast<mitk::PointSet*>(m_DataNode->GetData());
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

    if ( !m_UndoEnabled )
      delete doOp;

  }

  //reset the statemachine
  this->ResetStatemachineToStartState(timeStep);
}


void mitk::PointSetInteractor::InitAccordingToNumberOfPoints()
{
  if (m_DataNode == NULL)
    return;

  mitk::PointSet *pointSet = dynamic_cast<mitk::PointSet*>(m_DataNode->GetData());
  if ( pointSet != NULL )
  {
    //resize the CurrentStateVector
    this->ExpandStartStateVector(pointSet->GetPointSetSeriesSize());

    for (unsigned int timestep = 0; timestep < pointSet->GetPointSetSeriesSize(); timestep++)
    {
      //go to new timestep
      this->UpdateTimeStep(timestep);

      int numberOfPoints = pointSet->GetSize( timestep );
      if (numberOfPoints == 0)
        continue; //pointset is empty
      else
      {
        //we have a set of loaded points. Deselect all points, because they are all set to selected when added!
        this->UnselectAll(timestep);

        if (numberOfPoints<m_N || m_N <= -1)//if less than specified or specified as unlimited
        {
          //get the currentState to state "SpaceLeft"
          const mitk::Event nullEvent(NULL, Type_User, BS_NoButton, BS_NoButton, Key_none);
          mitk::StateEvent newStateEvent(EIDSMALLERN, &nullEvent);
          this->HandleEvent( &newStateEvent );
        }
        else if (numberOfPoints>=m_N)
        {
          if (numberOfPoints>m_N)
          {
            STATEMACHINE_WARN<<"Point Set contains more points than needed!\n";//display a warning that there are too many points
          }
          //get the currentState to state "Set full"
          const mitk::Event nullEvent(NULL, Type_User, BS_NoButton, BS_NoButton, Key_none);
          mitk::StateEvent newStateEvent(EIDEQUALSN, &nullEvent);
          this->HandleEvent( &newStateEvent );
        }
      }
    }
  }
  return;
}

void mitk::PointSetInteractor::DataChanged()
{
  this->InitAccordingToNumberOfPoints();
  return;
}
