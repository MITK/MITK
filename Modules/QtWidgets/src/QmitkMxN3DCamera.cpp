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
    self->m_InteractionStarted = true;
    return;
  }

  if (!self->m_InteractionStarted)
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

bool QmitkMxN3DCamera::ApplyViewFrame(vtkRenderer* renderer, const mitk::PlaneGeometry& plane)
{
  if (nullptr == renderer)
  {
    return false;
  }
  auto* camera = renderer->GetActiveCamera();

  auto normal = plane.GetNormal();
  normal.Normalize();
  auto viewUp = plane.GetAxisVector(1);
  viewUp.Normalize();

  const double directionOfProjection[3] = { -normal[0], -normal[1], -normal[2] };
  if (SameDirection(camera->GetDirectionOfProjection(), directionOfProjection)
      && SameDirection(camera->GetViewUp(), viewUp.GetDataPointer()))
  {
    return false;
  }

  double focalPoint[3];
  camera->GetFocalPoint(focalPoint);
  const double distance = camera->GetDistance();
  camera->SetPosition(focalPoint[0] + normal[0] * distance,
                      focalPoint[1] + normal[1] * distance,
                      focalPoint[2] + normal[2] * distance);
  camera->SetViewUp(viewUp[0], viewUp[1], viewUp[2]);
  camera->OrthogonalizeViewUp();
  renderer->ResetCameraClippingRange();
  return true;
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
