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

#include "mitkPointSnapInteractor.h"
#include <mitkPointSet.h>
#include <mitkPointOperation.h>
#include <mitkPositionEvent.h>
#include <mitkStateEvent.h>
#include <mitkOperationEvent.h>
#include <mitkUndoController.h>
#include <mitkDataTreeNode.h>
#include <mitkInteractionConst.h>
#include <mitkAction.h>
#include <mitkProperties.h>

mitk::PointSnapInteractor::PointSnapInteractor(const char * type, DataTreeNode* dataTreeNode)
: HierarchicalInteractor(type, dataTreeNode), m_Position(0)
{
  m_PointInteractor = mitk::PointInteractor::New("pointinteractor",dataTreeNode);
  this->AddInteractor((Interactor::Pointer)m_PointInteractor);
}

mitk::PointSnapInteractor::~PointSnapInteractor()
{}

void mitk::PointSnapInteractor::DeselectAllPoints()
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

void mitk::PointSnapInteractor::SelectPoint(int position)
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

float mitk::PointSnapInteractor::CalculateJurisdiction(StateEvent const* stateEvent) const
//call method from lower Interactors
{
  return this->CalculateLowerJurisdiction(stateEvent);
}


bool mitk::PointSnapInteractor::ExecuteAction( Action* action, mitk::StateEvent const* stateEvent )
{
  bool ok = false;//for return type bool

  //checking corresponding Data; has to be a PointSet or a subclass
  mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(m_DataTreeNode->GetData());
  if (pointSet == NULL)
    return false;

  /*Each case must watch the type of the event!*/
  switch (action->GetActionId())
  {
  case AcDESELECTALL:
    this->DeselectAllPoints();
    ok = true;
    break;
  case AcCHECKPOINT :
    /*checking if the Point transmitted is close enough to one point.*/
    {
      mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
      if (posEvent != NULL)
      {
        mitk::Point3D worldPoint = posEvent->GetWorldPosition();

        int PRECISION = 5;
        mitk::IntProperty *precision = dynamic_cast<IntProperty*>(action->GetProperty("precision"));
        if (precision != NULL)
        {
          PRECISION = precision->GetValue();
        }

        int position = pointSet->SearchPoint(worldPoint, PRECISION);
        if (position >=0)//point found at the current mouseposition
        {
          m_Position = position;
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDYES, posEvent);
          //call HandleEvent to leave the guard-state
          this->HandleEvent( newStateEvent );
          ok = true;
          break;
        }
      }
      //if no point found, or not a PositionEvent
      //new Event with information NO
      mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, posEvent);
      this->HandleEvent( newStateEvent );
      ok = true;
      break;
    }
  case AcSELECTPOINT:
    //now select the point found at the stored position
    {
      mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
      if (posEvent == NULL) 
        return false;
      mitk::Point3D worldPoint = pointSet->GetPoint(m_Position);

      PointOperation* doOp = new mitk::PointOperation(OpSELECTPOINT, worldPoint, m_Position);

      //Undo
      if (m_UndoEnabled)	//write to UndoMechanism
      {
        PointOperation* undoOp = new mitk::PointOperation(OpDESELECTPOINT, worldPoint, m_Position);
        OperationEvent *operationEvent = new OperationEvent(pointSet, doOp, undoOp);
        m_UndoController->SetOperationEvent(operationEvent);
      }

      //execute the Operation
      pointSet->ExecuteOperation(doOp);
      ok = true;
    }
    break;
  default:
    return Superclass::ExecuteAction( action, stateEvent );
  }

  return ok;
}

