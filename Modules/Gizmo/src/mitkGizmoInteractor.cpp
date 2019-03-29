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

#include "mitkGizmoInteractor.h"
#include "mitkGizmoMapper2D.h"

// MITK includes
#include <mitkInteractionConst.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkInternalEvent.h>
#include <mitkLookupTableProperty.h>
#include <mitkOperationEvent.h>
#include <mitkRotationOperation.h>
#include <mitkScaleOperation.h>
#include <mitkSurface.h>
#include <mitkUndoController.h>
#include <mitkVtkMapper.h>

// VTK includes
#include <vtkCamera.h>
#include <vtkInteractorObserver.h>
#include <vtkInteractorStyle.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkVector.h>
#include <vtkVectorOperators.h>

mitk::GizmoInteractor::GizmoInteractor()
{
    m_ColorForHighlight[0] = 1.0;
    m_ColorForHighlight[1] = 0.5;
    m_ColorForHighlight[2] = 0.0;
    m_ColorForHighlight[3] = 1.0;

    // TODO if we want to get this configurable, the this is the recipe:
    // - make the 2D mapper add corresponding properties to control "enabled" and "color"
    // - make the interactor evaluate those properties
    // - in an ideal world, modify the state machine on the fly and skip mouse move handling
}

mitk::GizmoInteractor::~GizmoInteractor()
{
}

void mitk::GizmoInteractor::ConnectActionsAndFunctions()
{
  CONNECT_CONDITION("PickedHandle", HasPickedHandle);

  CONNECT_FUNCTION("DecideInteraction", DecideInteraction);
  CONNECT_FUNCTION("MoveAlongAxis", MoveAlongAxis);
  CONNECT_FUNCTION("RotateAroundAxis", RotateAroundAxis);
  CONNECT_FUNCTION("MoveFreely", MoveFreely);
  CONNECT_FUNCTION("ScaleEqually", ScaleEqually);
  CONNECT_FUNCTION("FeedUndoStack", FeedUndoStack);
}

void mitk::GizmoInteractor::SetGizmoNode(DataNode *node)
{
  DataInteractor::SetDataNode(node);

  m_Gizmo = dynamic_cast<Gizmo *>(node->GetData());

  // setup picking from just this object
  m_Picker.clear();
}

void mitk::GizmoInteractor::SetManipulatedObjectNode(DataNode *node)
{
  if (node && node->GetData())
  {
    m_ManipulatedObjectGeometry = node->GetData()->GetGeometry();
  }
}

bool mitk::GizmoInteractor::HasPickedHandle(const InteractionEvent *interactionEvent)
{
  auto positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr ||
      m_Gizmo.IsNull() ||
      m_ManipulatedObjectGeometry.IsNull() ||
      interactionEvent->GetSender()->GetRenderWindow()->GetNeverRendered())
  {
    return false;
  }

  if (interactionEvent->GetSender()->GetMapperID() == BaseRenderer::Standard2D)
  {
    m_PickedHandle = PickFrom2D(positionEvent);
  }
  else
  {
    m_PickedHandle = PickFrom3D(positionEvent);
  }

  UpdateHandleHighlight();

  return m_PickedHandle != Gizmo::NoHandle;
}

void mitk::GizmoInteractor::DecideInteraction(StateMachineAction *, InteractionEvent *interactionEvent)
{
  assert(m_PickedHandle != Gizmo::NoHandle);

  auto positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
  {
    return;
  }

  // if something relevant was picked, we calculate a number of
  // important points and axes for the upcoming geometry manipulations

  // note initial state
  m_InitialClickPosition2D = positionEvent->GetPointerPositionOnScreen();
  m_InitialClickPosition3D = positionEvent->GetPositionInWorld();

  auto renderer = positionEvent->GetSender()->GetVtkRenderer();
  renderer->SetWorldPoint(m_InitialClickPosition3D[0], m_InitialClickPosition3D[1], m_InitialClickPosition3D[2], 0);
  renderer->WorldToDisplay();
  m_InitialClickPosition2DZ = renderer->GetDisplayPoint()[2];

  m_InitialGizmoCenter3D = m_Gizmo->GetCenter();
  positionEvent->GetSender()->WorldToDisplay(m_InitialGizmoCenter3D, m_InitialGizmoCenter2D);

  m_InitialManipulatedObjectGeometry = m_ManipulatedObjectGeometry->Clone();

  switch ( m_PickedHandle ) {
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
  default:
      break;
  }
  m_AxisOfMovement.Normalize();
  m_AxisOfRotation = m_AxisOfMovement;

  // for translation: test whether the user clicked into the "object's real" axis direction
  //                  or into the other one
  Vector3D intendedAxis = m_InitialClickPosition3D - m_InitialGizmoCenter3D;

  if ( intendedAxis * m_AxisOfMovement < 0 ) {
      m_AxisOfMovement *= -1.0;
  }

  // for rotation: test whether the axis of rotation is more looking in the direction
  //               of the camera or in the opposite
  vtkCamera *camera = renderer->GetActiveCamera();
  vtkVector3d cameraDirection(camera->GetDirectionOfProjection());

  double angle_rad = vtkMath::AngleBetweenVectors(cameraDirection.GetData(), m_AxisOfRotation.GetDataPointer());

  if ( angle_rad < vtkMath::Pi() / 2.0 ) {
      m_AxisOfRotation *= -1.0;
  }

  InternalEvent::Pointer decision;
  switch (m_PickedHandle)
  {
    case Gizmo::MoveAlongAxisX:
    case Gizmo::MoveAlongAxisY:
    case Gizmo::MoveAlongAxisZ:
      decision = InternalEvent::New(interactionEvent->GetSender(), this, "StartTranslationAlongAxis");
      break;
    case Gizmo::RotateAroundAxisX:
    case Gizmo::RotateAroundAxisY:
    case Gizmo::RotateAroundAxisZ:
      decision = InternalEvent::New(interactionEvent->GetSender(), this, "StartRotationAroundAxis");
      break;
    case Gizmo::MoveFreely:
      decision = InternalEvent::New(interactionEvent->GetSender(), this, "MoveFreely");
      break;
    case Gizmo::ScaleX:
    case Gizmo::ScaleY:
    case Gizmo::ScaleZ:
      decision = InternalEvent::New(interactionEvent->GetSender(), this, "ScaleEqually");
      break;
    default:
      break;
  }

  interactionEvent->GetSender()->GetDispatcher()->QueueEvent(decision);
}

void mitk::GizmoInteractor::MoveAlongAxis(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
  {
    return;
  }

  Vector3D mouseMovement3D = positionEvent->GetPositionInWorld() - m_InitialClickPosition3D;
  double projectedMouseMovement3D = mouseMovement3D * m_AxisOfMovement;
  Vector3D movement3D = projectedMouseMovement3D * m_AxisOfMovement;

  ApplyTranslationToManipulatedObject(movement3D);
  RenderingManager::GetInstance()->ForceImmediateUpdateAll();
}

void mitk::GizmoInteractor::RotateAroundAxis(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
  {
    return;
  }

  Vector2D originalVector = m_InitialClickPosition2D - m_InitialGizmoCenter2D;
  Vector2D currentVector = positionEvent->GetPointerPositionOnScreen() - m_InitialGizmoCenter2D;

  originalVector.Normalize();
  currentVector.Normalize();

  double angle_rad = std::atan2(currentVector[1], currentVector[0]) - std::atan2(originalVector[1], originalVector[0]);

  ApplyRotationToManipulatedObject(vtkMath::DegreesFromRadians(angle_rad));
  RenderingManager::GetInstance()->ForceImmediateUpdateAll();
}

void mitk::GizmoInteractor::MoveFreely(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
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
  Vector3D movementITK(worldPointITK - m_InitialClickPosition3D);

  ApplyTranslationToManipulatedObject(movementITK);
  RenderingManager::GetInstance()->ForceImmediateUpdateAll();
}

void mitk::GizmoInteractor::ScaleEqually(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
  {
    return;
  }

  Point2D currentPosition2D = positionEvent->GetPointerPositionOnScreen();
  double relativeSize = (currentPosition2D - m_InitialGizmoCenter2D).GetNorm() /
                        (m_InitialClickPosition2D - m_InitialGizmoCenter2D).GetNorm();

  ApplyEqualScalingToManipulatedObject(relativeSize);
  RenderingManager::GetInstance()->ForceImmediateUpdateAll();
}

void mitk::GizmoInteractor::ApplyTranslationToManipulatedObject(const Vector3D &translation)
{
  assert(m_ManipulatedObjectGeometry.IsNotNull());

  auto manipulatedGeometry = m_InitialManipulatedObjectGeometry->Clone();
  m_FinalDoOperation.reset(new PointOperation(OpMOVE, translation));
  if (m_UndoEnabled)
  {
    m_FinalUndoOperation.reset(new PointOperation(OpMOVE, -translation));
  }

  manipulatedGeometry->ExecuteOperation(m_FinalDoOperation.get());
  m_ManipulatedObjectGeometry->SetIdentity();
  m_ManipulatedObjectGeometry->Compose(manipulatedGeometry->GetIndexToWorldTransform());
}

void mitk::GizmoInteractor::ApplyEqualScalingToManipulatedObject(double scalingFactor)
{
  assert(m_ManipulatedObjectGeometry.IsNotNull());
  auto manipulatedGeometry = m_InitialManipulatedObjectGeometry->Clone();

  m_FinalDoOperation.reset(new ScaleOperation(OpSCALE, scalingFactor - 1.0, m_InitialGizmoCenter3D));
  if (m_UndoEnabled)
  {
    m_FinalUndoOperation.reset(new ScaleOperation(OpSCALE, -(scalingFactor - 1.0), m_InitialGizmoCenter3D));
  }

  manipulatedGeometry->ExecuteOperation(m_FinalDoOperation.get());
  m_ManipulatedObjectGeometry->SetIdentity();
  m_ManipulatedObjectGeometry->Compose(manipulatedGeometry->GetIndexToWorldTransform());
}

void mitk::GizmoInteractor::ApplyRotationToManipulatedObject(double angle_deg)
{
  assert(m_ManipulatedObjectGeometry.IsNotNull());

  auto manipulatedGeometry = m_InitialManipulatedObjectGeometry->Clone();

  m_FinalDoOperation.reset(new RotationOperation(OpROTATE, m_InitialGizmoCenter3D, m_AxisOfRotation, angle_deg));
  if (m_UndoEnabled)
  {
    m_FinalUndoOperation.reset(new RotationOperation(OpROTATE, m_InitialGizmoCenter3D, m_AxisOfRotation, -angle_deg));
  }

  manipulatedGeometry->ExecuteOperation(m_FinalDoOperation.get());
  m_ManipulatedObjectGeometry->SetIdentity();
  m_ManipulatedObjectGeometry->Compose(manipulatedGeometry->GetIndexToWorldTransform());
}

void mitk::GizmoInteractor::FeedUndoStack(StateMachineAction *, InteractionEvent *)
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
    m_UndoController->SetOperationEvent(operationEvent);
  }
}

mitk::Gizmo::HandleType mitk::GizmoInteractor::PickFrom2D(const InteractionPositionEvent *positionEvent)
{
  BaseRenderer *renderer = positionEvent->GetSender();

  auto mapper = GetDataNode()->GetMapper(BaseRenderer::Standard2D);
  auto gizmo_mapper = dynamic_cast<GizmoMapper2D *>(mapper);
  auto &picker = m_Picker[renderer];

  if (picker == nullptr)
  {
    picker = vtkSmartPointer<vtkCellPicker>::New();
    picker->SetTolerance(0.005);

    if (gizmo_mapper)
    { // doing this each time is bizarre
      picker->AddPickList(gizmo_mapper->GetVtkProp(renderer));
      picker->PickFromListOn();
    }
  }

  auto displayPosition = positionEvent->GetPointerPositionOnScreen();
  picker->Pick(displayPosition[0], displayPosition[1], 0, positionEvent->GetSender()->GetVtkRenderer());

  vtkIdType pickedPointID = picker->GetPointId();
  if (pickedPointID == -1)
  {
    return Gizmo::NoHandle;
  }

  vtkPolyData *polydata = gizmo_mapper->GetVtkPolyData(renderer);

  if (polydata && polydata->GetPointData() && polydata->GetPointData()->GetScalars())
  {
    double dataValue = polydata->GetPointData()->GetScalars()->GetTuple1(pickedPointID);
    return m_Gizmo->GetHandleFromPointDataValue(dataValue);
  }

  return Gizmo::NoHandle;
}

mitk::Gizmo::HandleType mitk::GizmoInteractor::PickFrom3D(const InteractionPositionEvent *positionEvent)
{
  BaseRenderer *renderer = positionEvent->GetSender();
  auto &picker = m_Picker[renderer];
  if (picker == nullptr)
  {
    picker = vtkSmartPointer<vtkCellPicker>::New();
    picker->SetTolerance(0.005);
    auto mapper = GetDataNode()->GetMapper(BaseRenderer::Standard3D);
    auto vtk_mapper = dynamic_cast<VtkMapper *>(mapper);
    if (vtk_mapper)
    { // doing this each time is bizarre
      picker->AddPickList(vtk_mapper->GetVtkProp(renderer));
      picker->PickFromListOn();
    }
  }

  auto displayPosition = positionEvent->GetPointerPositionOnScreen();
  picker->Pick(displayPosition[0], displayPosition[1], 0, positionEvent->GetSender()->GetVtkRenderer());

  vtkIdType pickedPointID = picker->GetPointId();
  if (pickedPointID == -1)
  {
    return Gizmo::NoHandle;
  }

  // _something_ picked
  return m_Gizmo->GetHandleFromPointID(pickedPointID);
}

void mitk::GizmoInteractor::UpdateHandleHighlight()
{
  if (m_HighlightedHandle != m_PickedHandle) {

    auto node = GetDataNode();
    if (node == nullptr) return;

    auto base_prop = node->GetProperty("LookupTable");
    if (base_prop == nullptr) return;

    auto lut_prop = dynamic_cast<LookupTableProperty*>(base_prop);
    if (lut_prop == nullptr) return;

    auto lut = lut_prop->GetLookupTable();
    if (lut == nullptr) return;

    // Table size is expected to constructed as one entry per gizmo-part enum value
    assert(lut->GetVtkLookupTable()->GetNumberOfTableValues() > std::max(m_PickedHandle, m_HighlightedHandle));

    // Reset previously overwritten color
    if (m_HighlightedHandle != Gizmo::NoHandle)
    {
        lut->SetTableValue(m_HighlightedHandle, m_ColorReplacedByHighlight);
    }

    // Overwrite currently highlighted color
    if (m_PickedHandle != Gizmo::NoHandle)
    {
      lut->GetTableValue(m_PickedHandle, m_ColorReplacedByHighlight);
      lut->SetTableValue(m_PickedHandle, m_ColorForHighlight);
    }

    // Mark node modified to allow repaint
    node->Modified();
    RenderingManager::GetInstance()->RequestUpdateAll(RenderingManager::REQUEST_UPDATE_ALL);

    m_HighlightedHandle = m_PickedHandle;
  }
}
