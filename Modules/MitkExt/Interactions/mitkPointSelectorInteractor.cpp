/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 14:46:19 +0100 (Di, 11 Dez 2007) $
Version:   $Revision: 13129 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPointSelectorInteractor.h"
#include <mitkPointOperation.h>
#include <mitkPositionEvent.h>
#include <mitkOperationEvent.h>
//#include "mitkStatusBar.h"
#include <mitkDataTreeNode.h>
#include <mitkPointSet.h>
#include <mitkInteractionConst.h>
#include <mitkAction.h>
#include <mitkProperties.h>
#include <vtkLinearTransform.h>
#include <mitkUndoController.h>
#include <mitkStateEvent.h>
#include <mitkState.h>


mitk::PointSelectorInteractor::PointSelectorInteractor(const char * type, DataTreeNode* dataTreeNode)
: Interactor(type, dataTreeNode), m_LastPosition(0)
{
  m_LastPoint.Fill(0);
}

mitk::PointSelectorInteractor::~PointSelectorInteractor()
{}

void mitk::PointSelectorInteractor::SelectPoint(int position)
{
  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(m_DataTreeNode->GetData());
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

void mitk::PointSelectorInteractor::DeselectAllPoints()
{
  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(m_DataTreeNode->GetData());
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

float mitk::PointSelectorInteractor::CalculateJurisdiction(StateEvent const* stateEvent) const
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
  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(m_DataTreeNode->GetData());
  if (pointSet == NULL)
    return 0;


  //since we now have 3D picking in GlobalInteraction and all events send are DisplayEvents with 3D information,
  //we concentrate on 3D coordinates
  mitk::Point3D worldPoint = posEvent->GetWorldPosition();
  float p[3];
  itk2vtk(worldPoint, p);
  //transforming the Worldposition to local coordinatesystem
  m_DataTreeNode->GetData()->GetGeometry()->GetVtkTransform()->GetInverse()->TransformPoint(p, p);
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
      itkWarningMacro("Difficulties in calculating Jurisdiction. Check PointSelectorInteractor");
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


bool mitk::PointSelectorInteractor::ExecuteAction( Action* action, mitk::StateEvent const* stateEvent )
{
  bool ok = false;//for return type bool

  //checking corresponding Data; has to be a PointSet or a subclass
  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(m_DataTreeNode->GetData());
  if (pointSet == NULL)
    return false;

  //for reading on the points, Id's etc
  //mitk::PointSet::DataType *itkPointSet = pointSet->GetPointSet();
  //mitk::PointSet::PointsContainer *itkPoints = itkPointSet->GetPoints();

  /*Each case must watch the type of the event!*/
  switch (action->GetActionId())
  {
  case AcADDPOINT:
    break;
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
    break;
  case AcMOVESELECTED://moves the point
    break;
  case AcFINISHMOVEMENT:
    break;
  case AcCHECKGREATERONE:
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
    break;
  default:
    return Superclass::ExecuteAction( action, stateEvent );
  }

  return ok;

}


