/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkMonaiLabel2DTool.h"
#include <mitkIOUtil.h>
#include <mitkImageReadAccessor.h>
#include <mitkSegTool2D.h>

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

void mitk::MonaiLabel2DTool::Activated()
{
  Superclass::Activated();
  this->SetLabelTransferScope(LabelTransferScope::AllLabels);
  this->KeepActiveAfterAcceptOn();
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
  return "MONAI Label 2D";
}

void mitk::MonaiLabel2DTool::WriteImage(const Image *inputAtTimeStep, const std::string &inputImagePath) const
{
  auto extendedImage = Image::New();
  std::array<unsigned int, 3> dim = {inputAtTimeStep->GetDimension(0), inputAtTimeStep->GetDimension(1), 1};
  mitk::PixelType pt = inputAtTimeStep->GetPixelType();
  extendedImage->Initialize(pt, 3, dim.data());

  ImageReadAccessor readAccessor(inputAtTimeStep);
  extendedImage->SetVolume(readAccessor.GetData());

  IOUtil::Save(extendedImage.GetPointer(), inputImagePath);
}

void mitk::MonaiLabel2DTool::WriteBackResults(MultiLabelSegmentation *previewImage,
                                              MultiLabelSegmentation *segResults,
                                              TimeStepType timeStep) const
 {
  if (segResults->GetTimeGeometry()->CountTimeSteps() > 1)
  {
    mitkThrow() << "Invalid time geometry found while writing back segmentation. "
                   " Expected static segmentation output from model.";
  }
  mitk::SegTool2D::WriteSliceToVolume(previewImage, this->GetWorkingPlaneGeometry(), segResults, timeStep, false);
 }
