/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMonaiLabel3DTool.h"

#include <mitkIOUtil.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usServiceReference.h>
#include <mitkImageReadAccessor.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, MonaiLabel3DTool, "MonaiLabel3D");
}

void mitk::MonaiLabel3DTool::Activated()
{
  Superclass::Activated();
  this->SetLabelTransferScope(LabelTransferScope::AllLabels);
}

const char **mitk::MonaiLabel3DTool::GetXPM() const
{
  return nullptr;
}

us::ModuleResource mitk::MonaiLabel3DTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("AI.svg");
  return resource;
}

const char *mitk::MonaiLabel3DTool::GetName() const
{
  return "MONAI Label 3D";
}

void mitk::MonaiLabel3DTool::WriteImage(const Image *inputAtTimeStep, const std::string &inputImagePath) const
{
  IOUtil::Save(inputAtTimeStep, inputImagePath);
}

void mitk::MonaiLabel3DTool::WriteBackResults(MultiLabelSegmentation *previewImage,
                                              MultiLabelSegmentation *segResults,
                                              TimeStepType timeStep) const
{
  if (segResults->GetTimeGeometry()->CountTimeSteps() > 1)
  {
    mitkThrow() << "Invalid time geometry found while writing back segmentation. "
                   " Expected static segmentation output from model.";
  }
  mitk::ImageReadAccessor newMitkImgAcc(segResults);
  previewImage->SetVolume(newMitkImgAcc.GetData(), timeStep);
}
