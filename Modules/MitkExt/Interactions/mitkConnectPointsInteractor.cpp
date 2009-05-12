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


#include "mitkConnectPointsInteractor.h"
#include <mitkLineOperation.h>
#include <mitkPositionEvent.h>
#include <mitkState.h>
#include <mitkStateEvent.h>
#include <mitkUndoController.h>
#include <mitkMesh.h>
#include <mitkDataTreeNode.h>
#include <mitkInteractionConst.h>
#include <mitkAction.h>
#include <vtkLinearTransform.h>

//how precise must the user pick the point
//default value
const int PRECISION = 5;

mitk::ConnectPointsInteractor::ConnectPointsInteractor(const char * type, DataTreeNode* dataTreeNode, int n)
:Interactor(type, dataTreeNode), m_N(n), m_CurrentCellId(0), m_Precision(PRECISION)
{
  m_LastPoint.Fill(0);
  m_SumVec.Fill(0);
}

mitk::ConnectPointsInteractor::~ConnectPointsInteractor()
{
}

void mitk::ConnectPointsInteractor::SetPrecision(unsigned int precision)
{
  m_Precision = precision;
}

//##Documentation
//## overwritten cause this class can handle it better!
float mitk::ConnectPointsInteractor::CalculateJurisdiction(StateEvent const* stateEvent) const
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
      itkWarningMacro("Difficulties in calculating Jurisdiction. Check PointInteractor");
      return 0;
    }
    
    //and now between 0,5 and 1
    returnValue = 0.5 + (returnValue / 2);

    return returnValue;
  }
  else //not found
  {
    return 0;
  }
}

bool mitk::ConnectPointsInteractor::ExecuteAction( Action* action, mitk::StateEvent const* stateEvent )
{
  bool ok = false;//for return type bool

  //checking corresponding Data; has to be a Mesh or a subclass
  mitk::Mesh* mesh = dynamic_cast<mitk::Mesh*>(m_DataTreeNode->GetData());
  if (mesh == NULL)
    return false;

  //for reading on the points, Id's etc
  //mitk::PointSet::DataType *itkpointSet = mesh->GetPointSet();
  //mitk::PointSet::PointsContainer *points = itkpointSet->GetPoints();//Warning Fix: not used!

  /*Each case must watch the type of the event!*/
  switch (action->GetActionId())
  {
  case AcDONOTHING:
    ok = true;
    break;
  case AcADDPOINT:
    {
      mitk::DisplayPositionEvent const  *posEvent = dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());
      if (posEvent == NULL) 
        return false;

      mitk::Point3D worldPoint;
      worldPoint = posEvent->GetWorldPosition();

      int position = mesh->SearchPoint(worldPoint, m_Precision);
      if (position >= 0)//found a point near enough to the given point
      {
        // if the point is the last in current cell, remove it (this has to be moved in a separate action)
        bool deleteLine=false;
        if(mesh->GetMesh()->GetCells()->Size() > 0) 
        {
          Mesh::CellAutoPointer cellAutoPointer;
          ok = mesh->GetMesh()->GetCell(m_CurrentCellId, cellAutoPointer);
          if(ok)
          {
            Mesh::PointIdIterator last = cellAutoPointer->PointIdsEnd();
            --last;
            int foundCell = mesh->SearchFirstCell( (unsigned long) position ); 
            if (foundCell != -1)//not found
              deleteLine = ((unsigned int)foundCell == m_CurrentCellId) && (*last == (unsigned int)position);
          }
        }
        if(deleteLine)
        {
          LineOperation* doOp = new mitk::LineOperation(OpDELETELINE, m_CurrentCellId, position);
          if (m_UndoEnabled)
          {
            LineOperation* undoOp = new mitk::LineOperation(OpADDLINE, m_CurrentCellId, position);
            OperationEvent *operationEvent = new OperationEvent(mesh, doOp, undoOp, "Delete line");
            m_UndoController->SetOperationEvent(operationEvent);
          }
          //execute the Operation
          mesh->ExecuteOperation(doOp );
        }
        else
        {          
          // add new cell if necessary
          if(mesh->GetNewCellId() == 0) //allow single line only
            //allow multiple lines:          if((mesh->SearchFirstCell(position) >= 0) || ((m_CurrentCellId == 0) && (mesh->GetNewCellId() == 0)))
          {
            //get the next cellId and set m_CurrentCellId
            m_CurrentCellId = mesh->GetNewCellId();

            //now reserv a new cell in m_ItkData
            LineOperation* doOp = new mitk::LineOperation(OpNEWCELL, m_CurrentCellId);
            if (m_UndoEnabled)
            {
              LineOperation* undoOp = new mitk::LineOperation(OpDELETECELL, m_CurrentCellId);
              OperationEvent *operationEvent = new OperationEvent(mesh, doOp, undoOp, "Add cell");
              m_UndoController->SetOperationEvent(operationEvent);
            }
            mesh->ExecuteOperation(doOp);
          }
          // add line if point is not yet included in current cell
          if(mesh->SearchFirstCell(position) < 0)
          {
            LineOperation* doOp = new mitk::LineOperation(OpADDLINE, m_CurrentCellId, position);
            if (m_UndoEnabled)
            {
              LineOperation* undoOp = new mitk::LineOperation(OpDELETELINE, m_CurrentCellId, position);
              OperationEvent *operationEvent = new OperationEvent(mesh, doOp, undoOp, "Add line");
              m_UndoController->SetOperationEvent(operationEvent);
            }
            //execute the Operation
            mesh->ExecuteOperation(doOp );
          }
        }
      }
      ok = true;
      break;
    }
  case AcREMOVEPOINT:
    {
      //mitk::DisplayPositionEvent const  *posEvent = dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());
      //if (posEvent == NULL) 
      //  return false;

      //mitk::Point3D worldPoint;
      //worldPoint = posEvent->GetWorldPosition();

      //int position = mesh->SearchPoint(worldPoint, m_Precision);
      //if (position>=0)//found a point near enough to the given point
      //{
      //  // if the point is in the current cell, remove it (this has to be moved in a separate action)
      //  if(mesh->SearchFirstCell(position) == m_CurrentCellId)
      //  {
      //    LineOperation* doOp = new mitk::LineOperation(OpDELETELINE, m_CurrentCellId, position);
      //  if (m_UndoEnabled)
      //  {
      //   LineOperation* undoOp = new mitk::LineOperation(OpADDLINE, m_CurrentCellId, position);
      //   OperationEvent *operationEvent = new OperationEvent(mesh, doOp, undoOp);
      //   m_UndoController->SetOperationEvent(operationEvent);
      //  }
      //  //execute the Operation
      //    mesh->ExecuteOperation(doOp );
      //  }
      //}
      ok = true;
      break;
    }
  case AcCHECKELEMENT:
    /*checking if the Point transmitted is close enough to one point. Then generate a new event with the point and let this statemaschine handle the event.*/
    {
      mitk::DisplayPositionEvent const  *posEvent = dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());
      if (posEvent != NULL)
      {
        mitk::Point3D worldPoint = posEvent->GetWorldPosition();

        int position = mesh->SearchPoint(worldPoint, m_Precision);
        if (position>=0)//found a point near enough to the given point
        {
          PointSet::PointType pt = mesh->GetPoint(position);//get that point, the one meant by the user!
          mitk::Point2D displPoint;
          displPoint[0] = worldPoint[0]; displPoint[1] = worldPoint[1];
          //new Event with information YES and with the correct point
          mitk::PositionEvent const* newPosEvent = new mitk::PositionEvent(posEvent->GetSender(), Type_None, BS_NoButton, BS_NoButton, Key_none, displPoint, pt);
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDYES, newPosEvent);
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
        mitk::DisplayPositionEvent const  *disPosEvent = dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());
        if (disPosEvent != NULL)
        {//2d Koordinates for 3D Interaction; return false to redo the last statechange
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, posEvent);
          this->HandleEvent(newStateEvent );
          ok = true;
        }
      }
      break;
    }
  case AcCHECKNMINUS1://generate Events if the set will be full after the addition of the point or not.
    {
      if (m_N<0)//number of points not limited->pass on "Amount of points in Set is smaller then N-1"
      {
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDSTSMALERNMINUS1, stateEvent->GetEvent());
        this->HandleEvent( newStateEvent );
        ok = true;
      }
      else 
      {
        if (mesh->GetSize()<(m_N-1))
          //pointset after addition won't be full
        {
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDSTSMALERNMINUS1, stateEvent->GetEvent());
          this->HandleEvent( newStateEvent );
          ok = true;
        }
        else //(mesh->GetSize()>=(m_N-1))
          //after the addition of a point, the container will be full
        {
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDSTLARGERNMINUS1, stateEvent->GetEvent());
          this->HandleEvent(newStateEvent );
          ok = true;
        }//else
      }//else
    }
    break;
  case AcCHECKEQUALS1:
    {
      if (mesh->GetSize()<=1)//the number of points in the list is 1 (or smaler)
      {
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
        this->HandleEvent( newStateEvent );
        ok = true;
      }
      else //more than 1 points in list, so stay in the state!
      {
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
        this->HandleEvent(newStateEvent );
        ok = true;
      }
    }
    break;
  default:
    return Superclass::ExecuteAction( action, stateEvent );
    //mitk::StatusBar::GetInstance()->DisplayText("Message from mitkConnectPointsInteractor: I do not understand the Action!", 10000);
    //ok = false;
    //a false here causes the statemachine to undo its last statechange.
    //otherwise it will end up in a different state, but without done Action.
    //if a transition really has no Action, than call donothing
  }

  return ok;
}
