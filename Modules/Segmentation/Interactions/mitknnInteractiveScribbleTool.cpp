/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveScribbleTool.h>

mitk::nnInteractiveScribbleTool::nnInteractiveScribbleTool()
{
  this->DisableContourMarkers();
  this->SetSize(3);
  this->Disable();
}

mitk::nnInteractiveScribbleTool::~nnInteractiveScribbleTool()
{
}

us::ModuleResource mitk::nnInteractiveScribbleTool::GetCursorIconResource() const
{
  return us::ModuleResource();
}

us::ModuleResource mitk::nnInteractiveScribbleTool::GetIconResource() const
{
  return us::ModuleResource();
}

const char* mitk::nnInteractiveScribbleTool::GetName() const
{
  return "Scribble";
}

bool mitk::nnInteractiveScribbleTool::IsEligibleForAutoInit() const
{
  return false;
}

void mitk::nnInteractiveScribbleTool::Activate(const Color& color)
{
  this->SetEnable3DInterpolation(false);
  this->Activated();
  this->Enable();

  this->SetFeedbackContourColor(color);

  m_PaintingNode->SetColor(color);
  m_PaintingNode->SetBoolProperty("outline binary", false);
}

void mitk::nnInteractiveScribbleTool::Deactivate()
{
  this->Disable();
  this->Deactivated();
  this->SetEnable3DInterpolation(true);
}

void mitk::nnInteractiveScribbleTool::OnInvertLogic(StateMachineAction*, InteractionEvent*)
{
}
