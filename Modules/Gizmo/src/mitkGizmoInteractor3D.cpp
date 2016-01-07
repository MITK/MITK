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
    m_PointPicker = vtkSmartPointer<vtkPointPicker>::New();
    m_PointPicker->SetTolerance(0.005);
}

mitk::GizmoInteractor3D::~GizmoInteractor3D()
{
}

void mitk::GizmoInteractor3D::ConnectActionsAndFunctions()
{
  // **Conditions** that can be used in the state machine, to ensure that certain conditions are met, before actually executing an action
  CONNECT_CONDITION("PickedHandle", HasPickedHandle);

  // **Function** in the statmachine patterns also referred to as **Actions**
  CONNECT_FUNCTION("DecideInteraction", DecideInteraction);
  CONNECT_FUNCTION("MoveAlongAxis", MoveAlongAxis);
  CONNECT_FUNCTION("RotateAroundAxis", RotateAroundAxis);
  CONNECT_FUNCTION("MoveFreely", MoveFreely);
}

void mitk::GizmoInteractor3D::SetGizmoNode(DataNode* node)
{
  // the gizmo
  DataInteractor::SetDataNode(node);

  m_Gizmo = dynamic_cast<mitk::Gizmo*>(node->GetData());

  m_PointPicker->GetPickList()->RemoveAllItems();
  m_PointPicker->PickFromListOff();
}

void mitk::GizmoInteractor3D::SetManipulatedObjectNode(DataNode* node)
{
  m_ManipulatedObject = node;
}

bool mitk::GizmoInteractor3D::HasPickedHandle(const InteractionEvent* interactionEvent)
{
  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
  {
    return false;
  }

  DataNode::Pointer gizmoNode = this->GetDataNode();

  if (m_Gizmo.IsNull())
  {
    return false;
  }

  if (m_ManipulatedObject.IsNull())
  {
    return false;
  }

  if (interactionEvent->GetSender()->GetRenderWindow()->GetNeverRendered())
  {
    return false;
  }

  if ( !m_PointPicker->GetPickFromList() )
  {
    auto mapper = GetDataNode()->GetMapper(mitk::BaseRenderer::Standard3D);
    auto vtk_mapper = dynamic_cast<VtkMapper*>(mapper);
    if ( vtk_mapper )
    { // doing this each time is bizarre
      m_PointPicker->AddPickList(vtk_mapper->GetVtkProp(interactionEvent->GetSender()));
      m_PointPicker->PickFromListOn();
    }
  }

  auto displayPosition = positionEvent->GetPointerPositionOnScreen();
  m_PointPicker->Pick( displayPosition[0], displayPosition[1], 0,
                       interactionEvent->GetSender()->GetVtkRenderer() );

  vtkIdType pickedPointID = m_PointPicker->GetPointId();
  if (pickedPointID == -1)
  {
      return false;
  }

  // _something_ picked
  m_PickedHandle = m_Gizmo->GetHandleFromPointID(pickedPointID);

  if (m_PickedHandle != Gizmo::NoHandle)
  {
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

    m_InitialManipulatedObjectGeometry = m_ManipulatedObject->GetData()->GetGeometry()->Clone();

    renderer->SetWorldPoint(m_InitialGizmoCenter3D[0],
                            m_InitialGizmoCenter3D[1],
                            m_InitialGizmoCenter3D[2],
                            0);
    renderer->WorldToDisplay();
    m_InitialGizmoCenter2DZ = renderer->GetDisplayPoint()[2];

    switch (m_PickedHandle)
    {
      case Gizmo::MoveAlongAxisX:
      case Gizmo::RotateAroundAxisX:
        m_RelevantAxis = Gizmo::AxisX;
        m_AxisOfMovement = m_InitialManipulatedObjectGeometry->GetAxisVector(0);
        break;
      case Gizmo::MoveAlongAxisY:
      case Gizmo::RotateAroundAxisY:
        m_RelevantAxis = Gizmo::AxisY;
          m_AxisOfMovement = m_InitialManipulatedObjectGeometry->GetAxisVector(1);
        break;
      case Gizmo::MoveAlongAxisZ:
      case Gizmo::RotateAroundAxisZ:
        m_RelevantAxis = Gizmo::AxisZ;
          m_AxisOfMovement = m_InitialManipulatedObjectGeometry->GetAxisVector(2);
        break;
    }
    m_AxisOfMovement.Normalize();

    // test whether we go really into that direction or into the opposite one
    Vector3D intendedAxis = m_InitialClickPosition3D - m_InitialGizmoCenter3D;

    if ( intendedAxis * m_AxisOfMovement < 0 )
    {
      m_AxisOfMovement *= -1.0;
    }

    return true;
  }
  else
  {
    return false;
  }
}

void mitk::GizmoInteractor3D::DecideInteraction(StateMachineAction* action, InteractionEvent* interactionEvent)
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
  }

  interactionEvent->GetSender()->GetDispatcher()->QueueEvent(decision);
}

void mitk::GizmoInteractor3D::MoveAlongAxis(StateMachineAction*, InteractionEvent* interactionEvent)
{
  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
  {
    return;
  }

  Vector2D axisVector2D = m_InitialClickPosition2D - m_InitialGizmoCenter2D;

  Vector2D axisVector2DNormalized = axisVector2D;
  axisVector2DNormalized.Normalize();

  Vector2D movement2D = positionEvent->GetPointerPositionOnScreen() - m_InitialClickPosition2D;
  double relativeMovement = movement2D * axisVector2DNormalized; // projection

  Vector3D movement3D = relativeMovement * m_AxisOfMovement;

  ApplyTranslationToManipulatedObject(movement3D);
  positionEvent->GetSender()->ForceImmediateUpdate();
}

void mitk::GizmoInteractor3D::RotateAroundAxis(StateMachineAction*, InteractionEvent* interactionEvent)
{
    std::cout << "Rotate around axis " << m_RelevantAxis << std::endl;
}

void mitk::GizmoInteractor3D::MoveFreely(StateMachineAction*, InteractionEvent* interactionEvent)
{
  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
  {
    return;
  }

  Point2D currentPosition2D = positionEvent->GetPointerPositionOnScreen();

  auto renderer = positionEvent->GetSender()->GetVtkRenderer();
  renderer->SetDisplayPoint(currentPosition2D[0], currentPosition2D[1], m_InitialClickPosition2DZ);
  renderer->DisplayToWorld();
  vtkVector3d worldPointVTK(renderer->GetWorldPoint());
  Point3D worldPointITK(worldPointVTK.GetData());
  Vector3D movementITK( worldPointITK - m_InitialClickPosition3D );
  ApplyTranslationToManipulatedObject(movementITK);
  positionEvent->GetSender()->ForceImmediateUpdate();
}

void mitk::GizmoInteractor3D::ApplyTranslationToManipulatedObject(const Vector3D& translation)
{
  assert(m_ManipulatedObject.IsNotNull());

  auto manipulatedGeometry = m_InitialManipulatedObjectGeometry->GetIndexToWorldTransform()->Clone();

  manipulatedGeometry->Translate(translation);

  m_ManipulatedObject->GetData()->GetGeometry()->SetIndexToWorldTransform( manipulatedGeometry );
}
