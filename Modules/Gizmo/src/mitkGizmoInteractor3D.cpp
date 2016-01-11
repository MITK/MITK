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

#include "mitkGizmoInteractor3D.h"

#include <mitkInteractionConst.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkRotationOperation.h>
#include <mitkScaleOperation.h>
#include <mitkSurface.h>
#include <mitkVtkMapper.h>
#include <mitkInternalEvent.h>
#include <mitkOperationEvent.h>
#include <mitkUndoController.h>

#include <vtkVector.h>
#include <vtkVectorOperators.h>
#include <vtkMath.h>
#include <vtkCamera.h>
#include <vtkInteractorStyle.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorObserver.h>

mitk::GizmoInteractor3D::GizmoInteractor3D()
{
    m_Picker = vtkSmartPointer<vtkCellPicker>::New();
    m_Picker->SetTolerance(0.005);
}

mitk::GizmoInteractor3D::~GizmoInteractor3D()
{
}

void mitk::GizmoInteractor3D::ConnectActionsAndFunctions()
{
  CONNECT_CONDITION("PickedHandle", HasPickedHandle);

  CONNECT_FUNCTION("DecideInteraction", DecideInteraction);
  CONNECT_FUNCTION("MoveAlongAxis", MoveAlongAxis);
  CONNECT_FUNCTION("RotateAroundAxis", RotateAroundAxis);
  CONNECT_FUNCTION("MoveFreely", MoveFreely);
  CONNECT_FUNCTION("ScaleEqually", ScaleEqually);
  CONNECT_FUNCTION("FeedUndoStack", FeedUndoStack);
}

void mitk::GizmoInteractor3D::SetGizmoNode(DataNode* node)
{
  DataInteractor::SetDataNode(node);

  m_Gizmo = dynamic_cast<Gizmo*>(node->GetData());

  // setup picking from just this object
  m_Picker->GetPickList()->RemoveAllItems();
  m_Picker->PickFromListOff();
}

void mitk::GizmoInteractor3D::SetManipulatedObjectNode(DataNode* node)
{
  if (node && node->GetData())
  {
    m_ManipulatedObjectGeometry = node->GetData()->GetGeometry();
  }
}

bool mitk::GizmoInteractor3D::HasPickedHandle(const InteractionEvent* interactionEvent)
{
  auto positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
  {
    return false;
  }

  DataNode::Pointer gizmoNode = this->GetDataNode();

  if (m_Gizmo.IsNull())
  {
    return false;
  }

  if (m_ManipulatedObjectGeometry.IsNull())
  {
    return false;
  }

  if (interactionEvent->GetSender()->GetRenderWindow()->GetNeverRendered())
  {
    return false;
  }

  if ( !m_Picker->GetPickFromList() )
  {
    auto mapper = GetDataNode()->GetMapper(BaseRenderer::Standard3D);
    auto vtk_mapper = dynamic_cast<VtkMapper*>(mapper);
    if ( vtk_mapper )
    { // doing this each time is bizarre
      m_Picker->AddPickList(vtk_mapper->GetVtkProp(interactionEvent->GetSender()));
      m_Picker->PickFromListOn();
    }
  }

  auto displayPosition = positionEvent->GetPointerPositionOnScreen();
  m_Picker->Pick( displayPosition[0], displayPosition[1], 0,
                       interactionEvent->GetSender()->GetVtkRenderer() );

  vtkIdType pickedPointID = m_Picker->GetPointId();
  if (pickedPointID == -1)
  {
      return false;
  }

  // _something_ picked
  m_PickedHandle = m_Gizmo->GetHandleFromPointID(pickedPointID);
  if (m_PickedHandle != Gizmo::NoHandle)
  {
    // if something relevant was picked, we calculate a number of
    // important points and axes for the upcoming geometry manipulations

    // note initial state
    m_InitialClickPosition2D = positionEvent->GetPointerPositionOnScreen();
    m_InitialClickPosition3D = positionEvent->GetPositionInWorld();

    auto renderer = positionEvent->GetSender()->GetVtkRenderer();
    renderer->SetWorldPoint(m_InitialClickPosition3D[0],
                            m_InitialClickPosition3D[1],
                            m_InitialClickPosition3D[2],
                            0);
    renderer->WorldToDisplay();
    m_InitialClickPosition2DZ = renderer->GetDisplayPoint()[2];


    m_InitialGizmoCenter3D = m_Gizmo->GetCenter();
    positionEvent->GetSender()->WorldToDisplay( m_InitialGizmoCenter3D, m_InitialGizmoCenter2D );

    m_InitialManipulatedObjectGeometry = m_ManipulatedObjectGeometry->Clone();

    switch (m_PickedHandle)
    {
      case Gizmo::MoveAlongAxisX:
      case Gizmo::RotateAroundAxisX:
      case Gizmo::ScaleX:
        m_AxisOfMovement = m_InitialManipulatedObjectGeometry->GetAxisVector(0);
        break;
      case Gizmo::MoveAlongAxisY:
      case Gizmo::RotateAroundAxisY:
      case Gizmo::ScaleY:
        m_AxisOfMovement = m_InitialManipulatedObjectGeometry->GetAxisVector(1);
        break;
      case Gizmo::MoveAlongAxisZ:
      case Gizmo::RotateAroundAxisZ:
      case Gizmo::ScaleZ:
        m_AxisOfMovement = m_InitialManipulatedObjectGeometry->GetAxisVector(2);
        break;
    }
    m_AxisOfMovement.Normalize();
    m_AxisOfRotation = m_AxisOfMovement;

    // for translation: test whether the user clicked into the "object's real" axis direction
    //                  or into the other one
    Vector3D intendedAxis = m_InitialClickPosition3D - m_InitialGizmoCenter3D;

    if ( intendedAxis * m_AxisOfMovement < 0 )
    {
      m_AxisOfMovement *= -1.0;
    }

    // for rotation: test whether the axis of rotation is more looking in the direction
    //               of the camera or in the opposite
    vtkCamera* camera = renderer->GetActiveCamera();
    vtkVector3d cameraDirection( camera->GetDirectionOfProjection() );

    double angle_rad = vtkMath::AngleBetweenVectors( cameraDirection.GetData(),
                                                     m_AxisOfRotation.GetDataPointer() );

    if ( angle_rad < vtkMath::Pi() / 2.0 )
    {
        m_AxisOfRotation *= -1.0;
    }

    return true;
  }
  else
  {
    return false;
  }
}

void mitk::GizmoInteractor3D::DecideInteraction(StateMachineAction* action,
                                                InteractionEvent* interactionEvent)
{
  assert(m_PickedHandle != Gizmo::NoHandle);

  InternalEvent::Pointer decision;
  switch (m_PickedHandle)
  {
    case Gizmo::MoveAlongAxisX:
    case Gizmo::MoveAlongAxisY:
    case Gizmo::MoveAlongAxisZ:
      decision = InternalEvent::New(interactionEvent->GetSender(), this,
                                    "StartTranslationAlongAxis");
      break;
    case Gizmo::RotateAroundAxisX:
    case Gizmo::RotateAroundAxisY:
    case Gizmo::RotateAroundAxisZ:
      decision = InternalEvent::New(interactionEvent->GetSender(), this,
                                    "StartRotationAroundAxis");
      break;
    case Gizmo::MoveFreely:
      decision = InternalEvent::New(interactionEvent->GetSender(), this,
                                    "MoveFreely");
      break;
    case Gizmo::ScaleX:
    case Gizmo::ScaleY:
    case Gizmo::ScaleZ:
      decision = InternalEvent::New(interactionEvent->GetSender(), this,
                                    "ScaleEqually");
      break;
  }

  interactionEvent->GetSender()->GetDispatcher()->QueueEvent(decision);
}

void mitk::GizmoInteractor3D::MoveAlongAxis(StateMachineAction*, InteractionEvent* interactionEvent)
{
  auto positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
  {
    return;
  }

  Vector2D axisVector2D = m_InitialClickPosition2D - m_InitialGizmoCenter2D;
  axisVector2D.Normalize();

  Vector2D movement2D = positionEvent->GetPointerPositionOnScreen() - m_InitialClickPosition2D;
  double relativeMovement = movement2D * axisVector2D; // projection

  Vector3D movement3D = relativeMovement * m_AxisOfMovement;

  ApplyTranslationToManipulatedObject(movement3D);
  positionEvent->GetSender()->ForceImmediateUpdate();
}

void mitk::GizmoInteractor3D::RotateAroundAxis(StateMachineAction*,
                                               InteractionEvent* interactionEvent)
{
  auto positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
  {
    return;
  }

  Vector2D originalVector = m_InitialClickPosition2D - m_InitialGizmoCenter2D;
  Vector2D currentVector = positionEvent->GetPointerPositionOnScreen() - m_InitialGizmoCenter2D;

  originalVector.Normalize();
  currentVector.Normalize();

  double angle_rad = std::atan2(currentVector[1], currentVector[0]) -
                     std::atan2(originalVector[1], originalVector[0]);

  ApplyRotationToManipulatedObject(vtkMath::DegreesFromRadians(angle_rad));
  positionEvent->GetSender()->ForceImmediateUpdate();
}

void mitk::GizmoInteractor3D::MoveFreely(StateMachineAction*, InteractionEvent* interactionEvent)
{
  auto positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
  {
    return;
  }

  Point2D currentPosition2D = positionEvent->GetPointerPositionOnScreen();

  // re-use the initial z value to really move parallel to the camera plane
  auto renderer = positionEvent->GetSender()->GetVtkRenderer();
  renderer->SetDisplayPoint(currentPosition2D[0], currentPosition2D[1], m_InitialClickPosition2DZ);
  renderer->DisplayToWorld();
  vtkVector3d worldPointVTK(renderer->GetWorldPoint());
  Point3D worldPointITK(worldPointVTK.GetData());
  Vector3D movementITK( worldPointITK - m_InitialClickPosition3D );

  ApplyTranslationToManipulatedObject(movementITK);
  positionEvent->GetSender()->ForceImmediateUpdate();
}

void mitk::GizmoInteractor3D::ScaleEqually(StateMachineAction*, InteractionEvent* interactionEvent)
{
  auto positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
  {
    return;
  }

  Point2D currentPosition2D = positionEvent->GetPointerPositionOnScreen();
  double relativeSize = (currentPosition2D - m_InitialGizmoCenter2D).GetNorm()
                      / (m_InitialClickPosition2D - m_InitialGizmoCenter2D).GetNorm();

  ApplyEqualScalingToManipulatedObject(relativeSize);
  positionEvent->GetSender()->ForceImmediateUpdate();
}


void mitk::GizmoInteractor3D::ApplyTranslationToManipulatedObject(const Vector3D& translation)
{
  assert(m_ManipulatedObjectGeometry.IsNotNull());

  auto manipulatedGeometry = m_InitialManipulatedObjectGeometry->Clone();
  m_FinalDoOperation = std::make_unique<PointOperation>(OpMOVE, translation);
  if (m_UndoEnabled)
  {
    m_FinalUndoOperation = std::make_unique<PointOperation>(OpMOVE, -translation);
  }

  manipulatedGeometry->ExecuteOperation(m_FinalDoOperation.get());
  m_ManipulatedObjectGeometry->SetIdentity();
  m_ManipulatedObjectGeometry->Compose( manipulatedGeometry->GetIndexToWorldTransform() );
}

void mitk::GizmoInteractor3D::ApplyEqualScalingToManipulatedObject(double scalingFactor)
{
  assert(m_ManipulatedObjectGeometry.IsNotNull());
  auto manipulatedGeometry = m_InitialManipulatedObjectGeometry->Clone();

  m_FinalDoOperation = std::make_unique<ScaleOperation>(OpSCALE,
                                                        scalingFactor - 1.0,
                                                        m_InitialGizmoCenter3D);
  if (m_UndoEnabled)
  {
    m_FinalUndoOperation = std::make_unique<ScaleOperation>(OpSCALE,
                                                            -(scalingFactor - 1.0),
                                                            m_InitialGizmoCenter3D);
  }

  manipulatedGeometry->ExecuteOperation(m_FinalDoOperation.get());
  m_ManipulatedObjectGeometry->SetIdentity();
  m_ManipulatedObjectGeometry->Compose( manipulatedGeometry->GetIndexToWorldTransform() );
}

void mitk::GizmoInteractor3D::ApplyRotationToManipulatedObject(double angle_deg)
{
  assert(m_ManipulatedObjectGeometry.IsNotNull());

  auto manipulatedGeometry = m_InitialManipulatedObjectGeometry->Clone();

  m_FinalDoOperation = std::make_unique<RotationOperation>(OpROTATE,
                                                           m_InitialGizmoCenter3D,
                                                           m_AxisOfRotation,
                                                           angle_deg);
  if (m_UndoEnabled)
  {
    m_FinalUndoOperation = std::make_unique<RotationOperation>(OpROTATE,
                                                               m_InitialGizmoCenter3D,
                                                               m_AxisOfRotation,
                                                               -angle_deg);
  }

  manipulatedGeometry->ExecuteOperation(m_FinalDoOperation.get());
  m_ManipulatedObjectGeometry->SetIdentity();
  m_ManipulatedObjectGeometry->Compose( manipulatedGeometry->GetIndexToWorldTransform() );
}

void mitk::GizmoInteractor3D::FeedUndoStack(StateMachineAction*, InteractionEvent* interactionEvent)
{
  if (m_UndoEnabled)
  {
    OperationEvent *operationEvent = new OperationEvent(m_ManipulatedObjectGeometry,
                                                        // OperationEvent will destroy operations!
                                                        // --> release() and not get()
                                                        m_FinalDoOperation.release(),
                                                        m_FinalUndoOperation.release(),
                                                        "Direct geometry manipulation");
    mitk::OperationEvent::IncCurrObjectEventId(); // save each modification individually
    mitk::OperationEvent::ExecuteIncrement();
    m_UndoController->SetOperationEvent(operationEvent);
  }
}
