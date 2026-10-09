/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMxN3DCamera.h>

#include <mitkPlaneGeometry.h>
#include <mitkSlicedGeometry3D.h>

#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>

#include <cmath>
#include <utility>

vtkStandardNewMacro(QmitkMxN3DInteractorStyle);

namespace
{
  bool SameDirection(const double* a, const double* b)
  {
    return std::abs(a[0] - b[0]) < QmitkMxN3DCamera::Tolerance
           && std::abs(a[1] - b[1]) < QmitkMxN3DCamera::Tolerance
           && std::abs(a[2] - b[2]) < QmitkMxN3DCamera::Tolerance;
  }
}

QmitkMxN3DInteractorStyle::QmitkMxN3DInteractorStyle()
{
  auto observer = vtkSmartPointer<vtkCallbackCommand>::New();
  observer->SetCallback(&QmitkMxN3DInteractorStyle::OnInteractionEvent);
  observer->SetClientData(this);
  this->AddObserver(vtkCommand::StartInteractionEvent, observer);
  this->AddObserver(vtkCommand::InteractionEvent, observer);
  this->AddObserver(vtkCommand::EndInteractionEvent, observer);
}

QmitkMxN3DInteractorStyle::~QmitkMxN3DInteractorStyle() = default;

void QmitkMxN3DInteractorStyle::SetObservedRenderer(vtkRenderer* renderer)
{
  m_ObservedRenderer = renderer;
}

void QmitkMxN3DInteractorStyle::SetRotationCallback(RotationCallback callback)
{
  m_RotationCallback = std::move(callback);
}

void QmitkMxN3DInteractorStyle::SetRotationLock(RotationLock lock)
{
  m_RotationLock = std::move(lock);
}

void QmitkMxN3DInteractorStyle::SetCameraStepCallback(CameraStepCallback callback)
{
  m_CameraStepCallback = std::move(callback);
}

void QmitkMxN3DInteractorStyle::Rotate()
{
  if (m_RotationLock && m_RotationLock())
  {
    return;
  }
  Superclass::Rotate();
}

void QmitkMxN3DInteractorStyle::Spin()
{
  if (m_RotationLock && m_RotationLock())
  {
    return;
  }
  Superclass::Spin();
}

void QmitkMxN3DInteractorStyle::OnInteractionEvent(vtkObject*, unsigned long eventId, void* clientData, void*)
{
  auto* self = static_cast<QmitkMxN3DInteractorStyle*>(clientData);
  if (nullptr == self->m_ObservedRenderer)
  {
    return;
  }
  auto* camera = self->m_ObservedRenderer->GetActiveCamera();

  if (vtkCommand::StartInteractionEvent == eventId)
  {
    camera->GetDirectionOfProjection(self->m_StartDirectionOfProjection);
    camera->GetViewUp(self->m_StartViewUp);
    self->m_LastReportedState = QmitkMxN3DCamera::CaptureState(camera);
    self->m_InteractionStarted = true;
    return;
  }

  if (!self->m_InteractionStarted)
  {
    return;
  }
  self->ReportCameraStep();
  if (vtkCommand::InteractionEvent == eventId)
  {
    return;
  }
  self->m_InteractionStarted = false;

  if (!SameDirection(camera->GetDirectionOfProjection(), self->m_StartDirectionOfProjection)
      || !SameDirection(camera->GetViewUp(), self->m_StartViewUp))
  {
    if (self->m_RotationCallback)
    {
      self->m_RotationCallback();
    }
  }
}

void QmitkMxN3DInteractorStyle::ReportCameraStep()
{
  const auto current = QmitkMxN3DCamera::CaptureState(m_ObservedRenderer->GetActiveCamera());
  const auto& last = m_LastReportedState;
  const bool changed = (current.focalPoint - last.focalPoint).GetNorm() > QmitkMxN3DCamera::Tolerance
                       || std::abs(current.visibleHalfHeight - last.visibleHalfHeight) > QmitkMxN3DCamera::Tolerance
                       || !SameDirection(current.directionOfProjection.GetDataPointer(),
                                         last.directionOfProjection.GetDataPointer())
                       || !SameDirection(current.viewUp.GetDataPointer(), last.viewUp.GetDataPointer());
  if (!changed)
  {
    return;
  }
  const auto from = m_LastReportedState;
  m_LastReportedState = current;
  if (m_CameraStepCallback)
  {
    m_CameraStepCallback(from, current);
  }
}

bool QmitkMxN3DCamera::ApplyViewFrame(vtkRenderer* renderer, const mitk::PlaneGeometry& plane)
{
  auto normal = plane.GetNormal();
  normal.Normalize();
  auto viewUp = plane.GetAxisVector(1);
  viewUp.Normalize();
  return ApplyDirection(renderer, -normal, viewUp);
}

bool QmitkMxN3DCamera::ApplyDirection(vtkRenderer* renderer,
                                      const mitk::Vector3D& directionOfProjection,
                                      const mitk::Vector3D& viewUp)
{
  if (nullptr == renderer)
  {
    return false;
  }
  auto* camera = renderer->GetActiveCamera();

  auto direction = directionOfProjection;
  direction.Normalize();
  auto up = viewUp;
  up.Normalize();
  if (SameDirection(camera->GetDirectionOfProjection(), direction.GetDataPointer())
      && SameDirection(camera->GetViewUp(), up.GetDataPointer()))
  {
    return false;
  }

  double focalPoint[3];
  camera->GetFocalPoint(focalPoint);
  const double distance = camera->GetDistance();
  camera->SetPosition(focalPoint[0] - direction[0] * distance,
                      focalPoint[1] - direction[1] * distance,
                      focalPoint[2] - direction[2] * distance);
  camera->SetViewUp(up[0], up[1], up[2]);
  camera->OrthogonalizeViewUp();
  renderer->ResetCameraClippingRange();
  return true;
}

QmitkMxN3DInteractorStyle::CameraState QmitkMxN3DCamera::CaptureState(vtkCamera* camera)
{
  QmitkMxN3DInteractorStyle::CameraState state;
  camera->GetFocalPoint(state.focalPoint.GetDataPointer());
  state.visibleHalfHeight = VisibleHalfHeight(camera);
  camera->GetDirectionOfProjection(state.directionOfProjection.GetDataPointer());
  camera->GetViewUp(state.viewUp.GetDataPointer());
  return state;
}

double QmitkMxN3DCamera::VisibleHalfHeight(vtkCamera* camera)
{
  if (camera->GetParallelProjection())
  {
    return camera->GetParallelScale();
  }
  return camera->GetDistance() * std::tan(vtkMath::RadiansFromDegrees(camera->GetViewAngle()) / 2.0);
}

void QmitkMxN3DCamera::Zoom(vtkRenderer* renderer, double factor)
{
  if (nullptr == renderer || factor <= 0.0)
  {
    return;
  }
  auto* camera = renderer->GetActiveCamera();
  if (camera->GetParallelProjection())
  {
    camera->SetParallelScale(camera->GetParallelScale() / factor);
  }
  else
  {
    camera->Dolly(factor);
  }
  renderer->ResetCameraClippingRange();
}

void QmitkMxN3DCamera::Translate(vtkRenderer* renderer, const mitk::Vector3D& worldVector)
{
  if (nullptr == renderer)
  {
    return;
  }
  auto* camera = renderer->GetActiveCamera();
  double position[3];
  double focalPoint[3];
  camera->GetPosition(position);
  camera->GetFocalPoint(focalPoint);
  for (int i = 0; i < 3; ++i)
  {
    position[i] += worldVector[i];
    focalPoint[i] += worldVector[i];
  }
  camera->SetPosition(position);
  camera->SetFocalPoint(focalPoint);
  renderer->ResetCameraClippingRange();
}

mitk::Vector3D QmitkMxN3DCamera::PlaneToWorld(const mitk::PlaneGeometry& plane, const mitk::Vector2D& planeVector)
{
  auto axis0 = plane.GetAxisVector(0);
  axis0.Normalize();
  auto axis1 = plane.GetAxisVector(1);
  axis1.Normalize();
  return axis0 * planeVector[0] + axis1 * planeVector[1];
}

mitk::Vector2D QmitkMxN3DCamera::WorldToPlane(const mitk::PlaneGeometry& plane, const mitk::Vector3D& worldVector)
{
  auto axis0 = plane.GetAxisVector(0);
  axis0.Normalize();
  auto axis1 = plane.GetAxisVector(1);
  axis1.Normalize();
  mitk::Vector2D result;
  result[0] = worldVector * axis0;
  result[1] = worldVector * axis1;
  return result;
}

mitk::AnatomicalPlane QmitkMxN3DCamera::NearestStandardPlane(const mitk::BaseGeometry& geometry,
                                                             const mitk::Vector3D& directionOfProjection)
{
  struct Candidate
  {
    mitk::AnatomicalPlane plane;
    bool top;
    bool frontside;
    bool rotated;
  };
  // The flags 'SliceNavigationController::Update()' uses for each standard
  // plane, so the candidates are the planes a 2D cell would show.
  constexpr Candidate candidates[] = {
    { mitk::AnatomicalPlane::Axial, false, false, true },
    { mitk::AnatomicalPlane::Sagittal, true, true, false },
    { mitk::AnatomicalPlane::Coronal, false, true, false }
  };

  auto direction = directionOfProjection;
  direction.Normalize();

  auto nearest = mitk::AnatomicalPlane::Axial;
  double bestAlignment = -1.0;
  for (const auto& candidate : candidates)
  {
    const auto sliced = mitk::SlicedGeometry3D::New();
    sliced->InitializePlanes(&geometry, candidate.plane, candidate.top, candidate.frontside, candidate.rotated);
    auto normal = sliced->GetPlaneGeometry(0)->GetNormal();
    normal.Normalize();
    const double alignment = std::abs(direction * normal);
    if (alignment > bestAlignment)
    {
      bestAlignment = alignment;
      nearest = candidate.plane;
    }
  }
  return nearest;
}
