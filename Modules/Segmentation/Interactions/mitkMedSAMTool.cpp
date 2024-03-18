/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMedSAMTool.h"

#include "mitkImageAccessByItk.h"
#include "mitkToolManager.h"
#include <mitkSegTool2D.h>
// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

using namespace std::chrono_literals;

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, MedSAMTool, "MedSAMTool");
}

const char *mitk::MedSAMTool::GetName() const
{
  return "MedSAM";
}

void mitk::MedSAMTool::Activated()
{
  SegWithPreviewTool::Activated();
  this->SetLabelTransferScope(LabelTransferScope::ActiveLabel);
  this->SetLabelTransferMode(LabelTransferMode::MapLabel);
}

void mitk::MedSAMTool::ConnectActionsAndFunctions() {}

bool mitk::MedSAMTool::HasPicks() const
{
  return true; // check if bounding box is ready
}

std::stringstream mitk::MedSAMTool::GetPointsAsCSVString(const mitk::BaseGeometry * /*baseGeometry*/)
{
  std::stringstream pointsAndLabels;
  pointsAndLabels << "Coordinates\n";
  pointsAndLabels << "198 130 230 220";
  return pointsAndLabels;
}
