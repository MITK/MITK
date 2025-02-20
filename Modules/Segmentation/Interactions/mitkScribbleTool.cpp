/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkScribbleTool.h>

mitk::ScribbleTool::ScribbleTool()
{
  this->Disable();
}

mitk::ScribbleTool::~ScribbleTool()
{
}

us::ModuleResource mitk::ScribbleTool::GetCursorIconResource() const
{
  return us::ModuleResource();
}

us::ModuleResource mitk::ScribbleTool::GetIconResource() const
{
  return us::ModuleResource();
}

const char* mitk::ScribbleTool::GetName() const
{
  return "Scribble";
}

bool mitk::ScribbleTool::IsEligibleForAutoInit() const
{
  return false;
}

void mitk::ScribbleTool::Activate()
{
  this->Activated();
  this->Enable();
}

void mitk::ScribbleTool::Deactivate()
{
  this->Disable();
  this->Deactivated();
}

void mitk::ScribbleTool::OnInvertLogic(StateMachineAction*, InteractionEvent*)
{
}
