/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkAffineImageCropperInteractor.h"

#include "mitkInteractionConst.h"
#include "mitkInteractionPositionEvent.h"
#include "mitkRotationOperation.h"
#include "mitkSurface.h"
#include "mitkVtkMapper.h"

#include <mitkBoundingObject.h>
#include <mitkMouseWheelEvent.h>
#include <vtkCamera.h>
#include <vtkInteractorStyle.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRenderWindowInteractor.h>

#include <mitkRotationOperation.h>
#include <mitkScaleOperation.h>

mitk::AffineImageCropperInteractor::AffineImageCropperInteractor()
{
  m_OriginalGeometry = Geometry3D::New();
}

mitk::AffineImageCropperInteractor::~AffineImageCropperInteractor()
{
}

void mitk::AffineImageCropperInteractor::ConnectActionsAndFunctions()
{
  // **Conditions** that can be used in the state machine, to ensure that certain conditions are met, before actually
  // executing an action
  CONNECT_CONDITION("isOverObject", CheckOverObject);

  // **Function** in the statmachine patterns also referred to as **Actions**
  CONNECT_FUNCTION("selectObject", SelectObject);
  CONNECT_FUNCTION("deselectObject", DeselectObject);
  CONNECT_FUNCTION("initTranslate", InitTranslate);
  CONNECT_FUNCTION("initRotate", InitRotate);
  CONNECT_FUNCTION("initDeformation", InitDeformation);
  CONNECT_FUNCTION("translateObject", TranslateObject);
  CONNECT_FUNCTION("rotateObject", RotateObject);
  CONNECT_FUNCTION("deformObject", DeformObject);
  CONNECT_FUNCTION("scaleRadius", ScaleRadius);
}

void mitk::AffineImageCropperInteractor::DataNodeChanged()
{
}

bool mitk::AffineImageCropperInteractor::CheckOverObject(const InteractionEvent *interactionEvent)
{
  mitk::DataNode::Pointer dn = this->GetDataNode();
  if (dn.IsNull())
    return false;
  const auto *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  Point3D currentPickedPoint = positionEvent->GetPositionInWorld();
  auto *object = dynamic_cast<mitk::BoundingObject *>(dn->GetData());
  object->GetGeometry()->WorldToIndex(currentPickedPoint, currentPickedPoint);
  return object && object->GetGeometry()->GetBoundingBox()->IsInside(currentPickedPoint);
}

void mitk::AffineImageCropperInteractor::SelectObject(StateMachineAction *, InteractionEvent *)
{
  mitk::DataNode::Pointer dn = this->GetDataNode();

  if (dn.IsNull())
    return;

  m_SelectedNode = dn;

  RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::AffineImageCropperInteractor::Deselect()
{
  mitk::DataNode::Pointer dn = this->GetDataNode();

  if (dn.IsNull())
    return;

  m_SelectedNode = dn;
}

void mitk::AffineImageCropperInteractor::DeselectObject(StateMachineAction *, InteractionEvent *)
{
  Deselect();
  RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::AffineImageCropperInteractor::ScaleRadius(StateMachineAction *, InteractionEvent *interactionEvent)
{
  const auto *wheelEvent = dynamic_cast<const MouseWheelEvent *>(interactionEvent);
  if (wheelEvent == nullptr)
    return;

  if (m_SelectedNode.IsNull())
    return;

  double scale = (double)(wheelEvent->GetWheelDelta()) / 64.0;
  mitk::Point3D newScale;
  newScale[0] = newScale[1] = newScale[2] = scale;

  mitk::Point3D anchorPoint = wheelEvent->GetPositionInWorld();
  auto *doOp = new mitk::ScaleOperation(OpSCALE, newScale, anchorPoint);
  m_SelectedNode->GetData()->GetGeometry()->ExecuteOperation(doOp);

  RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::AffineImageCropperInteractor::InitTranslate(StateMachineAction *, InteractionEvent *interactionEvent)
{
  if (m_SelectedNode.IsNull())
    return;
  auto *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  m_InitialPickedPoint = positionEvent->GetPositionInWorld();
  m_InitialPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();

  mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface *>(m_SelectedNode->GetData());
  mitk::BaseGeometry::Pointer surGeo = surface->GetGeometry();
  m_InitialOrigin = surGeo->GetOrigin();
}

void mitk::AffineImageCropperInteractor::InitRotate(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  m_InitialPickedPoint = positionEvent->GetPositionInWorld();
  m_InitialPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();

  mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface *>(m_SelectedNode->GetData());
  mitk::BaseGeometry::Pointer surGeo = surface->GetGeometry();
  m_OriginalGeometry = dynamic_cast<mitk::Geometry3D *>(surGeo.GetPointer());
}

void mitk::AffineImageCropperInteractor::InitDeformation(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  m_InitialPickedPoint = positionEvent->GetPositionInWorld();
  m_InitialPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();

  mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface *>(m_SelectedNode->GetData());
  mitk::BaseGeometry::Pointer surGeo = surface->GetGeometry();
  m_OriginalGeometry = dynamic_cast<mitk::Geometry3D *>(surGeo.GetPointer());
}

void mitk::AffineImageCropperInteractor::TranslateObject(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  Point3D currentPickedPoint = positionEvent->GetPositionInWorld();

  Vector3D interactionMove;
  interactionMove[0] = currentPickedPoint[0] - m_InitialPickedPoint[0];
  interactionMove[1] = currentPickedPoint[1] - m_InitialPickedPoint[1];
  interactionMove[2] = currentPickedPoint[2] - m_InitialPickedPoint[2];

  mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface *>(m_SelectedNode->GetData());
  mitk::BaseGeometry::Pointer surGeo = surface->GetGeometry();
  surGeo->SetOrigin(m_InitialOrigin);
  surGeo->Translate(interactionMove);

  RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::AffineImageCropperInteractor::DeformObject(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  Point3D currentPickedPoint = positionEvent->GetPositionInWorld();
  Vector3D interactionMove = currentPickedPoint - m_InitialPickedPoint;

  mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface *>(m_SelectedNode->GetData());
  surface->SetGeometry(m_OriginalGeometry);
  mitk::BaseGeometry::Pointer surGeo = surface->GetGeometry();

  surGeo->WorldToIndex(interactionMove, interactionMove);
  Point3D scale;
  for (int i = 0; i < 3; ++i)
  {
    scale[i] = (interactionMove[i] * surGeo->GetMatrixColumn(i).magnitude()) - 1;
  }

  mitk::Point3D anchorPoint = surGeo->GetCenter();

  auto *doOp = new mitk::ScaleOperation(OpSCALE, scale, anchorPoint);
  surGeo->ExecuteOperation(doOp);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::AffineImageCropperInteractor::RotateObject(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  Point2D currentPickedDisplayPoint = positionEvent->GetPointerPositionOnScreen();
  if (currentPickedDisplayPoint.EuclideanDistanceTo(m_InitialPickedDisplayPoint) < 1)
    return;

  vtkRenderer *currentVtkRenderer = interactionEvent->GetSender()->GetVtkRenderer();

  if (currentVtkRenderer && currentVtkRenderer->GetActiveCamera())
  {
    double vpn[3];
    currentVtkRenderer->GetActiveCamera()->GetViewPlaneNormal(vpn);

    Vector3D rotationAxis;
    rotationAxis[0] = vpn[0];
    rotationAxis[1] = vpn[1];
    rotationAxis[2] = vpn[2];
    rotationAxis.Normalize();

    Vector2D move = currentPickedDisplayPoint - m_InitialPickedDisplayPoint;

    double rotationAngle = -57.3 * atan(move[0] / move[1]);
    if (move[1] < 0)
      rotationAngle += 180;

    // Use center of data bounding box as center of rotation
    Point3D rotationCenter = m_OriginalGeometry->GetCenter();
    if (positionEvent->GetSender()->GetMapperID() == BaseRenderer::Standard2D)
      rotationCenter = m_InitialPickedPoint;

    // Reset current Geometry3D to original state (pre-interaction) and
    // apply rotation
    RotationOperation op(OpROTATE, rotationCenter, rotationAxis, rotationAngle);
    Geometry3D::Pointer newGeometry = static_cast<Geometry3D *>(m_OriginalGeometry->Clone().GetPointer());
    newGeometry->ExecuteOperation(&op);
    m_SelectedNode->GetData()->SetGeometry(newGeometry);

    RenderingManager::GetInstance()->RequestUpdateAll();
  }
}
