/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkMonaiLabel2DTool.h"

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usServiceReference.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, MonaiLabel2DTool, "MonaiLabel2D");
}

mitk::MonaiLabel2DTool::MonaiLabel2DTool() 
{
  this->ResetsToEmptyPreviewOn();
  this->IsTimePointChangeAwareOff();
}

void mitk::MonaiLabel2DTool::Activated()
{
  Superclass::Activated();
  this->SetLabelTransferScope(LabelTransferScope::AllLabels);
  this->SetLabelTransferMode(LabelTransferMode::AddLabel);
}

const char **mitk::MonaiLabel2DTool::GetXPM() const
{
  return nullptr;
}

us::ModuleResource mitk::MonaiLabel2DTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("AI.svg");
  return resource;
}

const char *mitk::MonaiLabel2DTool::GetName() const
{
  return "MonaiLabel2D";
}
