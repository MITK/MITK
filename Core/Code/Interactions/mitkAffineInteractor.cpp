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


#include "mitkAffineInteractor.h"
#include "mitkInteractionConst.h"
#include "mitkDataTreeNode.h"
#include "mitkGeometry3D.h"
#include "mitkRotationOperation.h"
#include "mitkPointOperation.h"
#include "mitkPositionEvent.h"
#include "mitkStateEvent.h"
#include "mitkOperationEvent.h"
#include "mitkUndoController.h"
#include "mitkDisplayPositionEvent.h"
#include "vtkTransform.h"
#include "mitkVtkPropRenderer.h"
#include "mitkProperties.h"
#include <itkBoundingBox.h>
#include <itkFixedArray.h>
#include "mitkAction.h"

//#include "mitkBoundingObject.h"
#include "mitkRenderingManager.h"

#include <math.h>

#include <vtkWorldPointPicker.h>
#include <vtkPicker.h>
#include "mitkGlobalInteraction.h"
#include "mitkFocusManager.h"
#include "mitkEventMapper.h"
#include "vtkProp3D.h"
#include "mitkVtkInteractorCameraController.h"
#include <vtkInteractorObserver.h>
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include <vtkInteractorObserver.h>

#include <iostream>

mitk::AffineInteractor::AffineInteractor(const char * type, DataTreeNode* dataTreeNode)
: Interactor(type, dataTreeNode)
{
}

bool mitk::AffineInteractor::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent)
{
  bool ok = false;

  TimeSlicedGeometry* inputtimegeometry = GetData()->GetTimeSlicedGeometry();
  if (inputtimegeometry == NULL)
    return false;

  Geometry3D* geometry = inputtimegeometry->GetGeometry3D(m_TimeStep);

  mitk::DisplayPositionEvent const *event = dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());
  switch (action->GetActionId())
  {
  case AcCHECKELEMENT:
    {
      mitk::Point3D worldPoint = event->GetWorldPosition();
      /* now we have a worldpoint. check if it is inside our object and select/deselect it accordingly */
      mitk::BoolProperty::Pointer selected;
      mitk::ColorProperty::Pointer color;
      mitk::StateEvent* newStateEvent = NULL;

      selected = dynamic_cast<mitk::BoolProperty*>(m_DataTreeNode->GetProperty("selected"));

      if ( selected.IsNull() ) {
        selected = mitk::BoolProperty::New();
        m_DataTreeNode->GetPropertyList()->SetProperty("selected", selected);
      }

      color = dynamic_cast<mitk::ColorProperty*>(m_DataTreeNode->GetProperty("color"));

      if ( color.IsNull() ) {
        color = mitk::ColorProperty::New();
        m_DataTreeNode->GetPropertyList()->SetProperty("color", color);
      }

      if (this->CheckSelected(worldPoint, m_TimeStep))
      {
        newStateEvent = new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
        selected->SetValue(true);
        color->SetColor(1.0, 1.0, 0.0);
      }
      else
      {
        newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
        selected = mitk::BoolProperty::New(false);
        color->SetColor(0.0, 0.0, 1.0);

        /*
        mitk::BoundingObject* b = dynamic_cast<mitk::BoundingObject*>(m_DataTreeNode->GetData());
        if(b != NULL)
        {
          color = (b->GetPositive())? mitk::ColorProperty::New(0.0, 0.0, 1.0) : mitk::ColorProperty::New(1.0, 0.0, 0.0);  // if deselected, a boundingobject is colored according to its positive/negative state
        }
        else
          color = mitk::ColorProperty::New(1.0, 1.0, 1.0);   // if deselcted and no bounding object, color is white
        */
      }

      /* write new state (selected/not selected) to the property */      
      this->HandleEvent( newStateEvent );
      ok = true;
      break;
    }
  case AcADD:
    {
      mitk::Point3D worldPoint = event->GetWorldPosition();
      mitk::StateEvent* newStateEvent = NULL;
      if (this->CheckSelected(worldPoint, m_TimeStep))
      {
        newStateEvent = new mitk::StateEvent(EIDYES, event);
        m_DataTreeNode->GetPropertyList()->SetProperty("selected", mitk::BoolProperty::New(true));  // TODO: Generate an Select Operation and send it to the undo controller ?
      }
      else  // if not selected, do nothing (don't deselect)
      {
        newStateEvent = new mitk::StateEvent(EIDNO, event);
      }
      //call HandleEvent to leave the guard-state
      this->HandleEvent( newStateEvent );
      ok = true;
      break;
    }
  case AcTRANSLATESTART:
  case AcROTATESTART:
  case AcSCALESTART:
    {
      m_LastMousePosition = event->GetWorldPosition();
      ok = true;
      break;
    }
  case AcTRANSLATE:
    {
      mitk::Point3D newPosition;
      newPosition = event->GetWorldPosition();
      newPosition -=  m_LastMousePosition.GetVectorFromOrigin();        // compute difference between actual and last mouse position
      m_LastMousePosition = event->GetWorldPosition();               // save current mouse position as last position

      /* create operation with position difference */
      mitk::PointOperation* doOp = new mitk::PointOperation(OpMOVE, newPosition, 0); // Index is not used here
      if (m_UndoEnabled)  //write to UndoMechanism
      {
        mitk::Point3D oldPosition=geometry->GetCornerPoint(0);

        PointOperation* undoOp = new mitk::PointOperation(OpMOVE, oldPosition, 0);
        OperationEvent *operationEvent = new OperationEvent(geometry, doOp, undoOp);
        m_UndoController->SetOperationEvent(operationEvent);
      }
      /* execute the Operation */
      geometry->ExecuteOperation(doOp);
      ok = true;
      break;
    }
  case AcTRANSLATEEND:
    {
      m_UndoController->SetOperationEvent(new UndoStackItem("Move object"));
      break;
    }
  case AcROTATE:
    {
      mitk::Point3D p = event->GetWorldPosition();

      mitk::Vector3D newPosition = p.GetVectorFromOrigin();

      mitk::Point3D dataPosition = geometry->GetCenter();

      newPosition = newPosition - dataPosition.GetVectorFromOrigin();  // calculate vector from center of the data object to the current mouse position

      mitk::Vector3D startPosition = m_LastMousePosition.GetVectorFromOrigin() - dataPosition.GetVectorFromOrigin();  // calculate vector from center of the data object to the last mouse position

      /* calculate rotation axis (by calculating the cross produkt of the vectors) */
      mitk::Vector3D rotationaxis;   
      rotationaxis[0] =  startPosition[1] * newPosition[2] - startPosition[2] * newPosition[1];
      rotationaxis[1] =  startPosition[2] * newPosition[0] - startPosition[0] * newPosition[2];
      rotationaxis[2] =  startPosition[0] * newPosition[1] - startPosition[1] * newPosition[0];

      /* calculate rotation angle in degrees */
      mitk::ScalarType angle = atan2((mitk::ScalarType)rotationaxis.GetNorm(), (mitk::ScalarType) (newPosition * startPosition)) * (180/vnl_math::pi);
      m_LastMousePosition = p; // save current mouse position as last mouse position

      /* create operation with center of rotation, angle and axis and send it to the geometry and Undo controller */
      mitk::RotationOperation* doOp = new mitk::RotationOperation(OpROTATE, dataPosition, rotationaxis, angle);

      if (m_UndoEnabled)  //write to UndoMechanism
      {
        RotationOperation* undoOp = new mitk::RotationOperation(OpROTATE, dataPosition, rotationaxis, -angle);
        OperationEvent *operationEvent = new OperationEvent(geometry, doOp, undoOp);
        m_UndoController->SetOperationEvent(operationEvent);
      }
      /* execute the Operation */
      geometry->ExecuteOperation(doOp);
      ok = true;
      break;
    }
  case AcROTATEEND:
    {
      m_UndoController->SetOperationEvent(new UndoStackItem("Rotate object"));
      break;
    }
  case AcSCALE:
    {    
      mitk::Point3D p = event->GetWorldPosition();

      mitk::Vector3D v = p - m_LastMousePosition;    
      /* calculate scale changes */
      mitk::Point3D newScale;
      newScale[0] = (geometry->GetAxisVector(0) * v) / geometry->GetExtentInMM(0);  // Scalarprodukt of normalized Axis
      newScale[1] = (geometry->GetAxisVector(1) * v) / geometry->GetExtentInMM(1);  // and direction vector of mouse movement
      newScale[2] = (geometry->GetAxisVector(2) * v) / geometry->GetExtentInMM(2);  // is the length of the movement vectors
      // projection onto the axis
      /* convert movement to local object coordinate system and mirror it to the positive quadrant */
      Vector3D start;
      Vector3D end;
      mitk::ScalarType convert[3];
      itk2vtk(m_LastMousePosition, convert);
      geometry->GetVtkTransform()->GetInverse()->TransformPoint(convert, convert);  // transform start point to local object coordinates
      start[0] = fabs(convert[0]);  start[1] = fabs(convert[1]);  start[2] = fabs(convert[2]);  // mirror it to the positive quadrant
      itk2vtk(p, convert);
      geometry->GetVtkTransform()->GetInverse()->TransformPoint(convert, convert);  // transform end point to local object coordinates
      end[0] = fabs(convert[0]);  end[1] = fabs(convert[1]);  end[2] = fabs(convert[2]);  // mirror it to the positive quadrant

      /* check if mouse movement is towards or away from the objects axes and adjust scale factors accordingly */
      Vector3D vLocal = start - end;
      newScale[0] = (vLocal[0] > 0.0) ? -fabs(newScale[0]) : +fabs(newScale[0]);
      newScale[1] = (vLocal[1] > 0.0) ? -fabs(newScale[1]) : +fabs(newScale[1]);
      newScale[2] = (vLocal[2] > 0.0) ? -fabs(newScale[2]) : +fabs(newScale[2]);

      m_LastMousePosition = p;  // update lastPosition for next mouse move

      /* generate Operation and send it to the receiving geometry */
      PointOperation* doOp = new mitk::PointOperation(OpSCALE, newScale, 0); // Index is not used here
      if (m_UndoEnabled)  //write to UndoMechanism
      {
        mitk::Point3D oldScaleData;
        oldScaleData[0] = -newScale[0];
        oldScaleData[1] = -newScale[1];
        oldScaleData[2] = -newScale[2];

        PointOperation* undoOp = new mitk::PointOperation(OpSCALE, oldScaleData, 0);
        OperationEvent *operationEvent = new OperationEvent(geometry, doOp, undoOp);
        m_UndoController->SetOperationEvent(operationEvent);
      }
      /* execute the Operation */
      geometry->ExecuteOperation(doOp);
      /* Update Volume Property with new value */
      /*
      mitk::BoundingObject* b = dynamic_cast<mitk::BoundingObject*>(m_DataTreeNode->GetData());
      if (b != NULL)
      {
        m_DataTreeNode->GetPropertyList()->SetProperty("volume", FloatProperty::New(b->GetVolume()));
        //std::cout << "Volume of Boundingobject is " << b->GetVolume()/1000.0 << " ml" << std::endl;
      }
      */
      ok = true;
      break;
    }
  case AcSCALEEND:
    {
      m_UndoController->SetOperationEvent(new UndoStackItem("Scale object"));
      break;
    }
  default:
    ok = Superclass::ExecuteAction(action, stateEvent);//, objectEventId, groupEventId);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  return ok;
}

bool mitk::AffineInteractor::CheckSelected(const mitk::Point3D& worldPoint, int timestep )
{
  bool selected = false;
  if (m_DataTreeNode->GetBoolProperty("selected", selected) == false)        // if property does not exist
    m_DataTreeNode->SetProperty("selected", mitk::BoolProperty::New(false));  // create it

  // check if mouseclick has hit the object
  /*
  mitk::BoundingObject::Pointer boundingObject = dynamic_cast<mitk::BoundingObject*>(m_DataTreeNode->GetData());
  if(boundingObject.IsNotNull())  // if it is a bounding object, use its inside function for exact hit calculation
  {
    selected = boundingObject->IsInside(worldPoint); // check if point is inside the object
  }
  else    // use the data objects bounding box to determine if hit
  */
  {
    const Geometry3D* geometry = GetData()->GetUpdatedTimeSlicedGeometry()->GetGeometry3D( timestep );
    selected = geometry->IsInside(worldPoint);
  }
  return selected;
}

bool mitk::AffineInteractor::ConvertDisplayEventToWorldPosition(mitk::DisplayPositionEvent const* displayEvent, mitk::Point3D& worldPoint)
{
  mitk::Point2D displayPoint = displayEvent->GetDisplayPosition();
  /* Copied from vtk Sphere widget */
  double focalPoint[4], position[4];
  double z;

  FocusManager::FocusElement* fe = mitk::GlobalInteraction::GetInstance()->GetFocus();
  mitk::VtkPropRenderer* glRenderer = dynamic_cast<mitk::VtkPropRenderer*>( fe );
  if ( glRenderer == NULL )
  {
    return false;
  }

  vtkRenderer *renderer = glRenderer->GetVtkRenderer();
  vtkCamera *camera = renderer->GetActiveCamera();
  if ( !camera )
  {
    return false;
  }
  // Compute the two points defining the motion vector
  camera->GetFocalPoint(focalPoint);
  //this->ComputeWorldToDisplay(focalPoint[0], focalPoint[1], focalPoint[2], focalPoint);
  renderer->SetWorldPoint(focalPoint[0], focalPoint[1], focalPoint[2], 1.0);
  renderer->WorldToDisplay();
  renderer->GetDisplayPoint(focalPoint);
  z = focalPoint[2];
  //      this->ComputeDisplayToWorld(displayPoint.x, displayPoint.y, z, position);
  renderer->SetDisplayPoint(displayPoint[0], displayPoint[1], z);
  renderer->DisplayToWorld();
  renderer->GetWorldPoint(position);
  if (position[3])
  {
    worldPoint[0] = position[0] / position[3];
    worldPoint[1] = position[1] / position[3];
    worldPoint[2] = position[2] / position[3];
    position[3] = 1.0;
  }
  else
  {
    worldPoint[0] = position[0];
    worldPoint[1] = position[1];
    worldPoint[2] = position[2];
  }
  return true;
}
