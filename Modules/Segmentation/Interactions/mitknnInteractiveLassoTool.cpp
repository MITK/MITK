/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveLassoTool.h>

mitk::LassoEvent::LassoEvent(ContourModel* contour)
  : m_Contour(contour)
{
}

mitk::LassoEvent::LassoEvent(const Self& other)
  : Superclass(other),
    m_Contour(other.m_Contour)
{
}

mitk::LassoEvent::~LassoEvent() = default;

const char* mitk::LassoEvent::GetEventName() const
{
  return "LassoEvent";
}

bool mitk::LassoEvent::CheckEvent(const itk::EventObject* event) const
{
  return dynamic_cast<const Self*>(event) != nullptr;
}

itk::EventObject* mitk::LassoEvent::MakeObject() const
{
  return new Self(*this);
}

mitk::ContourModel* mitk::LassoEvent::GetContour() const
{
  return m_Contour;
}

mitk::nnInteractiveLassoTool::nnInteractiveLassoTool()
{
  this->DisableContourMarkers();
  this->Disable();
}

mitk::nnInteractiveLassoTool::~nnInteractiveLassoTool()
{
}

us::ModuleResource mitk::nnInteractiveLassoTool::GetCursorIconResource() const
{
  return us::ModuleResource();
}

us::ModuleResource mitk::nnInteractiveLassoTool::GetIconResource() const
{
  return us::ModuleResource();
}

const char* mitk::nnInteractiveLassoTool::GetName() const
{
  return "Lasso";
}

bool mitk::nnInteractiveLassoTool::IsEligibleForAutoInit() const
{
  return false;
}

void mitk::nnInteractiveLassoTool::Activate(const Color& color)
{
  this->SetEnable3DInterpolation(false);
  this->Activated();
  this->Enable();

  this->SetFeedbackContourColor(color);
  this->SetFeedbackContourWidth(3.0f);
}

void mitk::nnInteractiveLassoTool::Deactivate()
{
  this->Disable();
  this->Deactivated();
  this->SetEnable3DInterpolation(true);
}

void mitk::nnInteractiveLassoTool::OnMouseReleased(StateMachineAction* action, InteractionEvent* event)
{
  auto contour = this->GetFeedbackContour()->Clone();

  if (contour->GetNumberOfVertices() < 3)
    return;

  Superclass::OnMouseReleased(action, event);
  this->InvokeEvent(LassoEvent(contour));
  this->GetWorkingDataNode()->GetDataAs<LabelSetImage>()->ClearBuffer();
}

void mitk::nnInteractiveLassoTool::OnInvertLogic(StateMachineAction*, InteractionEvent*)
{
}
