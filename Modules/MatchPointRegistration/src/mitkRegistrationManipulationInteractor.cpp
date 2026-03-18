/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkRegistrationManipulationInteractor.h"

#include <mitkApplicationCursor.h>
#include <mitkDisplayActionEventBroadcast.h>
#include <mitkInteractionEventObserver.h>
#include <mitkInteractionPositionEvent.h>

#include <usGetModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkRenderer.h>

#include <cmath>

namespace mitk
{
  itkEventMacroDefinition(RegistrationTranslationEvent, itk::AnyEvent);
  itkEventMacroDefinition(RegistrationRotationEvent, itk::AnyEvent);
  itkEventMacroDefinition(RegistrationScaleEvent, itk::AnyEvent);
  itkEventMacroDefinition(RegistrationSelectPositionEvent, itk::AnyEvent);
}

mitk::RegistrationManipulationInteractor::RegistrationManipulationInteractor()
{
  m_CenterOfRotation.Fill(0.0);
  m_TranslationDelta.Fill(0.0);
  m_RotationDelta.Axis.Fill(0.0);
  m_RotationDelta.AngleDeg = 0.0;
  m_SelectPosition.Fill(0.0);
}

mitk::RegistrationManipulationInteractor::~RegistrationManipulationInteractor()
{
}

void mitk::RegistrationManipulationInteractor::SetCenterOfRotation(const Point3D& center)
{
  m_CenterOfRotation = center;
}

void mitk::RegistrationManipulationInteractor::SetScalingEnabled(bool enabled)
{
  m_ScalingEnabled = enabled;
}

bool mitk::RegistrationManipulationInteractor::GetScalingEnabled() const
{
  return m_ScalingEnabled;
}

const mitk::Vector3D& mitk::RegistrationManipulationInteractor::GetTranslationDelta() const
{
  return m_TranslationDelta;
}

const mitk::RegistrationManipulationInteractor::RotationDelta&
mitk::RegistrationManipulationInteractor::GetRotationDelta() const
{
  return m_RotationDelta;
}

double mitk::RegistrationManipulationInteractor::GetScaleFactor() const
{
  return m_ScaleFactor;
}

const mitk::Point3D& mitk::RegistrationManipulationInteractor::GetSelectPosition() const
{
  return m_SelectPosition;
}

void mitk::RegistrationManipulationInteractor::ConnectActionsAndFunctions()
{
  CONNECT_CONDITION("ScalingEnabled", ScalingEnabled);

  CONNECT_FUNCTION("InitTranslation", InitTranslation);
  CONNECT_FUNCTION("Translate", Translate);
  CONNECT_FUNCTION("EndTranslation", EndTranslation);

  CONNECT_FUNCTION("InitRotation", InitRotation);
  CONNECT_FUNCTION("Rotate", Rotate);
  CONNECT_FUNCTION("EndRotation", EndRotation);

  CONNECT_FUNCTION("InitScaling", InitScaling);
  CONNECT_FUNCTION("Scale", Scale);
  CONNECT_FUNCTION("EndScaling", EndScaling);

  CONNECT_FUNCTION("SelectPosition", SelectPosition);
}

bool mitk::RegistrationManipulationInteractor::ScalingEnabled(const InteractionEvent*)
{
  return m_ScalingEnabled;
}

// --- Translation ---

void mitk::RegistrationManipulationInteractor::InitTranslation(StateMachineAction*, InteractionEvent* interactionEvent)
{
  const auto* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  this->PushCursorFromResource("Cursors/RegManip_Translate_Cursor.svg");

  m_InitialClickPosition2D = positionEvent->GetPointerPositionOnScreen();

  // Derive Z-depth from the MITK-picked world position, but then reproject via
  // DisplayToWorld so the initial position is computed identically to Translate().
  // Using GetPositionInWorld() directly as initial 3D anchor causes a jump on the
  // first move because MITK's pick and VTK's DisplayToWorld use slightly different
  // projection paths.
  const Point3D worldPick = positionEvent->GetPositionInWorld();
  auto renderer = positionEvent->GetSender()->GetVtkRenderer();
  renderer->SetWorldPoint(worldPick[0], worldPick[1], worldPick[2], 0);
  renderer->WorldToDisplay();
  m_InitialClickPosition2DZ = renderer->GetDisplayPoint()[2];

  // Now compute the canonical initial 3D position the same way Translate() will
  renderer->SetDisplayPoint(m_InitialClickPosition2D[0], m_InitialClickPosition2D[1], m_InitialClickPosition2DZ);
  renderer->DisplayToWorld();
  const double* w = renderer->GetWorldPoint();
  m_InitialClickPosition3D[0] = w[0];
  m_InitialClickPosition3D[1] = w[1];
  m_InitialClickPosition3D[2] = w[2];
}

void mitk::RegistrationManipulationInteractor::Translate(StateMachineAction*, InteractionEvent* interactionEvent)
{
  const auto* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  const Point2D currentPosition2D = positionEvent->GetPointerPositionOnScreen();

  // Convert to world coordinates using the stored Z-depth to stay in the camera plane
  auto renderer = positionEvent->GetSender()->GetVtkRenderer();
  renderer->SetDisplayPoint(currentPosition2D[0], currentPosition2D[1], m_InitialClickPosition2DZ);
  renderer->DisplayToWorld();
  const double* worldPoint = renderer->GetWorldPoint();
  const Point3D currentPosition3D(worldPoint);

  m_TranslationDelta = currentPosition3D - m_InitialClickPosition3D;

  // Reset initial position for incremental deltas
  m_InitialClickPosition3D = currentPosition3D;
  m_InitialClickPosition2D = currentPosition2D;

  this->InvokeEvent(RegistrationTranslationEvent());
}

void mitk::RegistrationManipulationInteractor::EndTranslation(StateMachineAction*, InteractionEvent*)
{
  this->PopCursorSafe();
  m_TranslationDelta.Fill(0.0);
}

// --- Rotation ---

void mitk::RegistrationManipulationInteractor::InitRotation(StateMachineAction*, InteractionEvent* interactionEvent)
{
  const auto* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  this->PushCursorFromResource("Cursors/RegManip_Rotate_Cursor.svg");

  m_InitialClickPosition2D = positionEvent->GetPointerPositionOnScreen();

  // Project center of rotation to screen coordinates
  positionEvent->GetSender()->WorldToDisplay(m_CenterOfRotation, m_CenterOfRotation2D);
}

void mitk::RegistrationManipulationInteractor::Rotate(StateMachineAction*, InteractionEvent* interactionEvent)
{
  const auto* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  const Point2D currentPosition2D = positionEvent->GetPointerPositionOnScreen();

  // Compute screen-space angle between initial and current vectors from center
  const Vector2D initialVector = m_InitialClickPosition2D - m_CenterOfRotation2D;
  const Vector2D currentVector = currentPosition2D - m_CenterOfRotation2D;

  const double initialAngle = std::atan2(initialVector[1], initialVector[0]);
  const double currentAngle = std::atan2(currentVector[1], currentVector[0]);

  const double angleDeltaRad = currentAngle - initialAngle;
  const double angleDeltaDeg = vtkMath::DegreesFromRadians(angleDeltaRad);

  // Rotation axis = view plane normal (camera direction of projection, flipped to point toward viewer)
  vtkCamera* camera = positionEvent->GetSender()->GetVtkRenderer()->GetActiveCamera();
  const double* dirOfProjection = camera->GetDirectionOfProjection();
  // The direction of projection points INTO the screen; we want the axis pointing toward
  // the viewer so that positive angles correspond to counterclockwise rotation on screen.
  Vector3D axis;
  axis[0] = -dirOfProjection[0];
  axis[1] = -dirOfProjection[1];
  axis[2] = -dirOfProjection[2];

  m_RotationDelta.Axis = axis;
  m_RotationDelta.AngleDeg = angleDeltaDeg;

  // Reset for incremental deltas
  m_InitialClickPosition2D = currentPosition2D;

  this->InvokeEvent(RegistrationRotationEvent());
}

void mitk::RegistrationManipulationInteractor::EndRotation(StateMachineAction*, InteractionEvent*)
{
  this->PopCursorSafe();
  m_RotationDelta.Axis.Fill(0.0);
  m_RotationDelta.AngleDeg = 0.0;
}

// --- Scaling ---

void mitk::RegistrationManipulationInteractor::InitScaling(StateMachineAction*, InteractionEvent* interactionEvent)
{
  const auto* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  this->PushCursorFromResource("Cursors/RegManip_Scale_Cursor.svg");

  m_InitialClickPosition2D = positionEvent->GetPointerPositionOnScreen();

  // Project center of rotation to screen coordinates
  positionEvent->GetSender()->WorldToDisplay(m_CenterOfRotation, m_CenterOfRotation2D);
}

void mitk::RegistrationManipulationInteractor::Scale(StateMachineAction*, InteractionEvent* interactionEvent)
{
  const auto* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  const Point2D currentPosition2D = positionEvent->GetPointerPositionOnScreen();

  const double initialDistance = (m_InitialClickPosition2D - m_CenterOfRotation2D).GetNorm();
  const double currentDistance = (currentPosition2D - m_CenterOfRotation2D).GetNorm();

  if (initialDistance > 1e-6)
  {
    m_ScaleFactor = currentDistance / initialDistance;
  }
  else
  {
    m_ScaleFactor = 1.0;
  }

  // Reset for next frame (incremental)
  m_InitialClickPosition2D = currentPosition2D;

  this->InvokeEvent(RegistrationScaleEvent());
}

void mitk::RegistrationManipulationInteractor::EndScaling(StateMachineAction*, InteractionEvent*)
{
  this->PopCursorSafe();
  m_ScaleFactor = 1.0;
}

// --- Position selection ---

void mitk::RegistrationManipulationInteractor::SelectPosition(StateMachineAction*, InteractionEvent* interactionEvent)
{
  const auto* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  m_SelectPosition = positionEvent->GetPositionInWorld();
  this->InvokeEvent(RegistrationSelectPositionEvent());
}

// --- Display interaction management ---

void mitk::RegistrationManipulationInteractor::DisableOriginalInteraction()
{
  if (m_OriginalInteractionDisabled)
    return;

  m_DisplayInteractionConfigs.clear();
  const auto eventObservers = us::GetModuleContext()->GetServiceReferences<InteractionEventObserver>();
  for (const auto& eventObserver : eventObservers)
  {
    auto* displayActionEventBroadcast = dynamic_cast<DisplayActionEventBroadcast*>(
      us::GetModuleContext()->GetService<InteractionEventObserver>(eventObserver));
    if (displayActionEventBroadcast != nullptr)
    {
      m_DisplayInteractionConfigs.insert(
        std::make_pair(eventObserver, displayActionEventBroadcast->GetEventConfig()));
      // Block LMB display interactions (pan, scroll, position-select) to prevent
      // conflict with our modifier+drag manipulation gestures. Our DataInteractor
      // handles all LMB events on the moving image, including plain click for
      // navigator position selection.
      displayActionEventBroadcast->AddEventConfig("DisplayConfigBlockLMB.xml");
    }
  }

  m_OriginalInteractionDisabled = true;
}

void mitk::RegistrationManipulationInteractor::EnableOriginalInteraction()
{
  if (!m_OriginalInteractionDisabled)
    return;

  for (const auto& displayInteractionConfig : m_DisplayInteractionConfigs)
  {
    if (displayInteractionConfig.first)
    {
      auto* displayActionEventBroadcast = static_cast<DisplayActionEventBroadcast*>(
        us::GetModuleContext()->GetService<InteractionEventObserver>(displayInteractionConfig.first));
      if (displayActionEventBroadcast != nullptr)
      {
        displayActionEventBroadcast->SetEventConfig(displayInteractionConfig.second);
      }
    }
  }

  m_DisplayInteractionConfigs.clear();
  m_OriginalInteractionDisabled = false;
}

// --- Cursor management ---

void mitk::RegistrationManipulationInteractor::PushCursorFromResource(const std::string& resourceName)
{
  auto* module = us::GetModuleContext()->GetModule();
  if (module == nullptr)
    return;

  us::ModuleResource resource = module->GetResource(resourceName);
  if (resource.IsValid())
  {
    us::ModuleResourceStream cursor(resource, std::ios::binary);
    ApplicationCursor::GetInstance()->PushCursor(cursor, 0, 0);
    ++m_NumPushedCursors;
  }
}

void mitk::RegistrationManipulationInteractor::PopCursorSafe()
{
  if (m_NumPushedCursors > 0)
  {
    ApplicationCursor::GetInstance()->PopCursor();
    --m_NumPushedCursors;
  }
}

void mitk::RegistrationManipulationInteractor::PushManipulationCursor()
{
  this->PushCursorFromResource("Cursors/RegManip_Base_Cursor.svg");
}

void mitk::RegistrationManipulationInteractor::PopManipulationCursor()
{
  // Pop all gesture-specific cursors that may still be on the stack, then the base cursor
  while (m_NumPushedCursors > 0)
  {
    this->PopCursorSafe();
  }
}
