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


#include "mitkPrimStripInteractor.h"
#include <mitkInteractionConst.h>
#include <mitkPositionEvent.h>
#include <mitkState.h>
#include <mitkStateEvent.h>
#include <mitkUndoController.h>
#include <mitkDataTreeNode.h>
#include <mitkMesh.h>
#include <vtkLinearTransform.h>
#include <mitkAction.h>
#include <mitkLineOperation.h>
#include <mitkProperties.h>
#include <mitkPointOperation.h>


mitk::PrimStripInteractor::PrimStripInteractor(const char * type, DataTreeNode* dataTreeNode)
: HierarchicalInteractor(type, dataTreeNode)
{
  m_LineInteractor = mitk::LineInteractor::New("lineinteractor",dataTreeNode);
  this->AddInteractor((Interactor::Pointer)m_LineInteractor);
}

mitk::PrimStripInteractor::~PrimStripInteractor()
{
  //delete m_LineInteractor;
}

float mitk::PrimStripInteractor::CalculateJurisdiction(StateEvent const* stateEvent) const
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
    return false;


  //go throgh all cells and get the BoundingBox:
  Mesh::ConstCellIterator cellIt = mesh->GetMesh()->GetCells()->Begin();
  Mesh::ConstCellIterator cellEnd = mesh->GetMesh()->GetCells()->End();
  while( cellIt != cellEnd )
  {

    Mesh::DataType::BoundingBoxPointer bBox = mesh->GetBoundingBoxFromCell( cellIt->Index() );
    if (bBox.IsNull())
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
    returnValue = worldPoint.EuclideanDistanceTo(center);
    
    //now compared to size of boundingbox to get between 0 and 1;
    returnValue = returnValue/( (bBox->GetMaximum().EuclideanDistanceTo(bBox->GetMinimum() ) ) );
    
    //shall be 1 if short length to center
    returnValue = 1 - returnValue;

    //check if the given position lies inside the data-object
    if (bBox->IsInside(worldPoint))
    {
      //mapped between 0,5 and 1
      returnValue = 0.5 + (returnValue / 2);
    }
    else
    {
      //set it in range between 0 and 0.5
      returnValue = returnValue / 2;
    }
    ++cellIt;
  }
  

//now check all lower statemachines:
  float lowerJurisdiction = this->CalculateLowerJurisdiction(stateEvent);
  
  if (returnValue < lowerJurisdiction)
    returnValue = lowerJurisdiction;

  return returnValue;
}


bool mitk::PrimStripInteractor::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent)
{
  bool ok = false;//for return type bool
  
  //checking corresponding Data; has to be a PointSet or a subclass
  mitk::Mesh* mesh = dynamic_cast<mitk::Mesh*>(m_DataTreeNode->GetData());
  if (mesh == NULL)
    return false;

  switch (action->GetActionId())
  {
  case AcINITNEWOBJECT:
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
    ok = true;
    break;
  case AcCHECKLINE:
    //check, if a line is hit
    {
      mitk::PositionEvent const  *posEvent = dynamic_cast <const mitk::PositionEvent *> (stateEvent->GetEvent());
      if (posEvent == NULL)
        return false;

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

      //var's for EvaluatePosition
      mitk::Point3D point = posEvent->GetWorldPosition();
      unsigned long cellId = 0;
      
      int precision = 5;
      mitk::IntProperty *precisionProperty = dynamic_cast<IntProperty*>(action->GetProperty("PRECISION"));
      if (precisionProperty != NULL)
      {
        precision = precisionProperty->GetValue();
      }

      //check wheather the mesh is hit.
      if (mesh->EvaluatePosition(point, cellId, precision))
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
        OperationEvent *operationEvent = new OperationEvent(m_DataTreeNode->GetData(), doOp, undoOp, "Move cell");
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
  case AcSETSTARTPOINT:
    {
      //get the selected point and assign it as startpoint
      Point3D point;//dummy
      point.Fill(0);
      int position = mesh->SearchSelectedPoint();
      if (position >-1)
      {
        PointOperation* doOp = new mitk::PointOperation(OpSETPOINTTYPE, point, position, true, PTSTART);

        //Undo
        if (m_UndoEnabled)  //write to UndoMechanism
        {
          PointOperation* undoOp = new mitk::PointOperation(OpSETPOINTTYPE, point, position, true, PTUNDEFINED);
          OperationEvent *operationEvent = new OperationEvent(mesh, doOp, undoOp, "Set start point");
          m_UndoController->SetOperationEvent(operationEvent);
        }

        //execute the Operation
        mesh->ExecuteOperation(doOp);
        ok = true;
      }
    }
    break;
  default:
    return Superclass::ExecuteAction(action, stateEvent);
  }
  return ok;
}
