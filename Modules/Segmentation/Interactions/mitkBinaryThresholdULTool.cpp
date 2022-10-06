/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBinaryThresholdULTool.h"

// us
#include "usGetModuleContext.h"
#include "usModule.h"
#include "usModuleContext.h"
#include "usModuleResource.h"

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, BinaryThresholdULTool, "ThresholdingUL tool");
}

mitk::BinaryThresholdULTool::BinaryThresholdULTool() : BinaryThresholdBaseTool()
{
}

mitk::BinaryThresholdULTool::~BinaryThresholdULTool()
{
}

const char **mitk::BinaryThresholdULTool::GetXPM() const
{
  return nullptr;
}

us::ModuleResource mitk::BinaryThresholdULTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("ULThreshold.svg");
  return resource;
}

const char *mitk::BinaryThresholdULTool::GetName() const
{
  return "UL Threshold";
}

