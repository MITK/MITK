/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkFillRegionTool.h"

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, FillRegionTool, "Fill tool");
}

const char **mitk::FillRegionTool::GetXPM() const
{
  return nullptr;
}

us::ModuleResource mitk::FillRegionTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Fill.svg");
  return resource;
}

us::ModuleResource mitk::FillRegionTool::GetCursorIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Fill_Cursor.svg");
  return resource;
}

const char *mitk::FillRegionTool::GetName() const
{
  return "Fill";
}

void mitk::FillRegionTool::PrepareFilling(const Image* /*workingSlice*/, Point3D /*seedPoint*/)
{
  auto labelSetImage = dynamic_cast<const LabelSetImage*>(this->GetWorkingData());

  if (nullptr == labelSetImage) mitkThrow() << "Invalid state of FillRegionTool. Working image is not of correct type.";

  m_FillLabelValue = labelSetImage->GetActiveLabel(labelSetImage->GetActiveLayer())->GetValue();
  m_MergeStyle = MultiLabelSegmentation::MergeStyle::Merge;
};

