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


#include "mitkLineInteractor.h"
#include "mitkStatusBar.h"
#include <mitkInteractionConst.h>
#include <mitkLineOperation.h>
#include <mitkPointOperation.h>
#include <mitkStateEvent.h>
#include <mitkOperationEvent.h>
#include <mitkUndoController.h>
#include <mitkPositionEvent.h>
#include <mitkStateTransitionOperation.h>
#include <mitkDataTreeNode.h>
#include <mitkMesh.h>
#include <mitkAction.h>
#include <mitkEventMapper.h>
#include <mitkLineOperation.h>
#include <mitkProperties.h>
#include <vtkLinearTransform.h>
#include <mitkLine.h>
#include <mitkPointSet.h>


mitk::LineInteractor::LineInteractor(const char * type, DataTreeNode* dataTreeNode)
: HierarchicalInteractor(type, dataTreeNode), m_CurrentLineId(0), m_CurrentCellId(0)
{
  m_PointInteractor = mitk::PointSnapInteractor::New("pointSNAPinteractor",NULL);
  this->AddInteractor((Interactor::Pointer)m_PointInteractor);
  m_LastPoint.Fill(0);
}

mitk::LineInteractor::~LineInteractor()
{
}

void mitk::LineInteractor::DeselectAllLines()//\*todo: move to mitkMesh.cpp
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
    for (unsigned int i = 0; i < selectedLines.size(); i++)
    {
      mitk::LineOperation* doOp = new mitk::LineOperation(OpDESELECTLINE, cellDataIt->Index(), -1, -1, selectedLines[i]);
      if (m_UndoEnabled)
      {
        mitk::LineOperation* undoOp = new mitk::LineOperation(OpSELECTLINE, cellDataIt->Index(), -1, -1, selectedLines[i]);
        OperationEvent *operationEvent = new OperationEvent(mesh, doOp, undoOp);
        m_UndoController->SetOperationEvent(operationEvent);
      }
      mesh->ExecuteOperation(doOp);
    }
  }
}

float mitk::LineInteractor::CalculateJurisdiction(StateEvent const* stateEvent) const
//go through all lines and check, if the given Point lies near a line
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
  mitk::Mesh* mesh = dynamic_cast<mitk::Mesh*>(m_DataTreeNode->GetData());
  if (mesh == NULL)
    return 0;


  //since we now have 3D picking in GlobalInteraction and all events send are DisplayEvents with 3D information,
  //we concentrate on 3D coordinates
  mitk::Point3D worldPoint = posEvent->GetWorldPosition();
  float p[3];
  itk2vtk(worldPoint, p);
  //transforming the Worldposition to local coordinatesystem
  m_DataTreeNode->GetData()->GetGeometry()->GetVtkTransform()->GetInverse()->TransformPoint(p, p);
  vtk2itk(p, worldPoint);

  //now check if the point lies on a line;
  unsigned long lineId = 0;
  unsigned long cellId = 0;

  float distance = 5;
  bool found =  mesh->SearchLine(worldPoint, distance, lineId, cellId);
  if (found)
    //how far away is the line from the point?
  {
    //get the cell
    Mesh::CellAutoPointer cellAutoPointer;
    if (!mesh->GetMesh()->GetCell(cellId,cellAutoPointer))
    {
      itkWarningMacro("Cell could not be found!Check LineInteractor");
      return 0;
    }
    //get the two points according to the recieved lineId
    mitk::PointSet::PointType pointA, pointB;
    mitk::Mesh::PointIdConstIterator pointIdIt = cellAutoPointer.GetPointer()->PointIdsBegin();
    mitk::Mesh::PointIdConstIterator pointIdEnd = cellAutoPointer.GetPointer()->PointIdsEnd();
    //get to the desired index of pointIndexes according to lineId
    //to get there, we need to iterate over the pointIds "lineId"-times!
    //then we have an iterator on the pointIds of the desired line
    unsigned int counter = 0;
    while (pointIdIt != pointIdEnd)
    {
      if (counter == lineId)
        break;
      ++counter;
      ++pointIdIt;
    }

    if ( ! mesh->GetMesh()->GetPoint((*pointIdIt), &pointA) )//get the PointA
      return 0;
    ++pointIdIt;
    if (pointIdIt == pointIdEnd)//then it was the last and the first point
      pointIdIt = cellAutoPointer.GetPointer()->PointIdsBegin();
    if ( ! mesh->GetMesh()->GetPoint((*pointIdIt), &pointB) )//get the PointB
      return 0;

    //now we have the points and so we can calculate the distance
    Line<PointSet::CoordinateType> *line = new Line<PointSet::CoordinateType>();
    line->SetPoints(pointA, pointB);

    double pointDistance = line->Distance(worldPoint);

    //because we searched for a line in the distance, we now can set it according to distance between 0 and 1
    returnValue = 1 - ( pointDistance / distance );

    //and now between 0,5 and 1
    returnValue = 0.5 + (returnValue / 2);
  }
  else //not found
  {
    returnValue = 0;
  }


  //now check all lower statemachines:
  float lowerJurisdiction = this->CalculateLowerJurisdiction(stateEvent);

  if (returnValue<lowerJurisdiction)
    returnValue = lowerJurisdiction;

  return returnValue;
}


bool mitk::LineInteractor::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent)
{
  bool ok = false;//for return type bool

  //has to be done here, cause otherwise different precisions are possible in CheckLine and CheckSelected;
  // this should not be, cause then strange interaction is possible! e.g. picking (checkLine) selects a line and picking it again to move it (checkselected) 
  //deselects it, cause the precision is lower!!! 
  int PRECISION = 5;
  mitk::IntProperty *precision = dynamic_cast<IntProperty*>(action->GetProperty("precision"));
  if (precision != NULL)
  {
    PRECISION = precision->GetValue();
  }

  //checking corresponding Data; has to be a PointSet or a subclass
  mitk::Mesh* mesh = dynamic_cast<mitk::Mesh*>(m_DataTreeNode->GetData());
  if (mesh == NULL)
    return false;

  switch (action->GetActionId())
  {
  case AcADDLINE:
    {
      Operation* doOp = new mitk::Operation(OpADDLINE);
      if (m_UndoEnabled)
      {
        Operation* undoOp = new mitk::Operation(OpDELETELINE);
        OperationEvent *operationEvent = new OperationEvent(mesh, doOp, undoOp, "Add line");
        m_UndoController->SetOperationEvent(operationEvent);
      }
      //execute the Operation
      mesh->ExecuteOperation(doOp );
    }
    ok = true;
    break;
  case AcDESELECTALL:
    {
      this->DeselectAllLines();//undo-supported able deselect of all points in the DataList
      ok = true;
    }
    break;
  case AcCHECKLINE:
    //checks if the position of the mouse lies over a line
    {
      mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
      if (posEvent != NULL)
      {
        mitk::Point3D worldPoint = posEvent->GetWorldPosition();

        unsigned long lineId, cellId;

        bool found = mesh->SearchLine(worldPoint, PRECISION, lineId, cellId);
        if (found)//found a point near enough to the given point
        {
          m_CurrentLineId = lineId;
          m_CurrentCellId = cellId;
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDYES, posEvent);
          //call HandleEvent to leave the guard-state
          this->HandleEvent( newStateEvent );
          ok = true;
        }
        else //not found
        {
          //new Event with information NO
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, posEvent);
          this->HandleEvent(newStateEvent );
          ok = true;
        }
      }
      else //not a positionEvent, so call warning and go on with EIDNO. that way we don't risk to hang up in a guard-state
      {
        itkWarningMacro("recieved wrong event-type! Check mitkLineOperation::AcCHECKLINE.");
        //new Event with information NO
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
        this->HandleEvent(newStateEvent );
        ok = true;
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

        bool found = mesh->SearchLine(worldPoint, PRECISION, lineId, cellId);
        if (found && 
          m_CurrentCellId == cellId &&
          m_CurrentLineId == lineId )//found the same line again
        {
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDYES, posEvent);
          //call HandleEvent to leave the guard-state
          this->HandleEvent( newStateEvent );
          ok = true;
        }
        else //not found
        {
          //new Event with information NO
          mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, posEvent);
          this->HandleEvent(newStateEvent );
          ok = true;
        }
      }
      else //not a positionEvent, so call warning and go on with EIDNO. that way we don't risk to hang up in a guard-state
      {
        itkWarningMacro("recieved wrong event-type! Check mitkLineOperation::AcCHECKLINE.");
        //new Event with information NO
        mitk::StateEvent* newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
        this->HandleEvent(newStateEvent );
        ok = true;
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
      if (m_UndoEnabled)  //write to UndoMechanism
      {
        LineOperation* undoOp = new mitk::LineOperation(OpDESELECTLINE, m_CurrentCellId, 0, 0, m_CurrentLineId);
        OperationEvent *operationEvent = new OperationEvent(mesh, doOp, undoOp);
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
      if (m_UndoEnabled)  //write to UndoMechanism
      {
        LineOperation* undoOp = new mitk::LineOperation(OpSELECTLINE, m_CurrentCellId, 0, 0, m_CurrentLineId);
        OperationEvent *operationEvent = new OperationEvent( mesh, doOp, undoOp );
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
          OperationEvent *operationEvent = new OperationEvent(m_DataTreeNode->GetData(), doOp, undoOp, "Move line");
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
    return Superclass::ExecuteAction( action, stateEvent );
  }
  return ok;
}
