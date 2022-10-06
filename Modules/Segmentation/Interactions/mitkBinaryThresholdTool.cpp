/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBinaryThresholdTool.h"

// us
#include "usGetModuleContext.h"
#include "usModule.h"
#include "usModuleResource.h"

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, BinaryThresholdTool, "Thresholding tool");
}

mitk::BinaryThresholdTool::BinaryThresholdTool() : BinaryThresholdBaseTool()
{
  LockedUpperThresholdOn();
}

mitk::BinaryThresholdTool::~BinaryThresholdTool()
{
}

const char **mitk::BinaryThresholdTool::GetXPM() const
{
  return nullptr;
}

us::ModuleResource mitk::BinaryThresholdTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Threshold.svg");
  return resource;
}

const char *mitk::BinaryThresholdTool::GetName() const
{
  return "Threshold";
}


void mitk::BinaryThresholdTool::SetThresholdValue(double value)
{
  this->SetThresholdValues(value, this->GetSensibleMaximumThreshold());
}
