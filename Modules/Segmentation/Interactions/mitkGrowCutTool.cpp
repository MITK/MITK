/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkGrowCutTool.h"
#include "mitkToolManager.h"
#include "mitkImageCast.h"
#include "mitkTool.h"
#include <mitkLabelSetImage.h>
#include <mitkLabelSetImageHelper.h>
#include "mitkGrowCutSegmentationFilter.h"

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

// ITK
#include <itkImage.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, GrowCutTool, "GrowCutTool");
}

mitk::GrowCutTool::GrowCutTool() : SegWithPreviewTool(false, "PressMoveReleaseAndPointSetting")
{
  this->ResetsToEmptyPreviewOn();
}

mitk::GrowCutTool::~GrowCutTool() {}

bool mitk::GrowCutTool::CanHandle(const BaseData *referenceData, const BaseData *workingData) const
{
  if (!Superclass::CanHandle(referenceData, workingData))
    return false;

  auto *image = dynamic_cast<const Image *>(referenceData);

  if (image == nullptr)
    return false;

  return true;
}

const char **mitk::GrowCutTool::GetXPM() const
{
  return nullptr;
}

const char *mitk::GrowCutTool::GetName() const
{
  return "GrowCut";
}

us::ModuleResource mitk::GrowCutTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("GrowCut.svg");
  return resource;
}

void mitk::GrowCutTool::Activated()
{
  Superclass::Activated();

  m_DistancePenalty = 0.0;
}

void mitk::GrowCutTool::Deactivated()
{
  Superclass::Deactivated();
}

bool mitk::GrowCutTool::HasMoreThanTwoSeedLabel()
{
  if (nullptr == this->GetToolManager()->GetWorkingData(0))
  {
    return false;
  }

  auto *workingDataLabelSetImage =
    dynamic_cast<mitk::LabelSetImage *>(this->GetToolManager()->GetWorkingData(0)->GetData());

  if (nullptr == workingDataLabelSetImage)
  {
    return false;
  }

  workingDataLabelSetImage->SetActiveLayer(0);
  auto numberOfLabels = workingDataLabelSetImage->GetNumberOfLabels();

  if (numberOfLabels >= 3)
  {
    return true;
  }

  return false;
}

void mitk::GrowCutTool::DoUpdatePreview(const Image *inputAtTimeStep,
                                        const Image * /*oldSegAtTimeStep*/,
                                        LabelSetImage *previewImage,
                                        TimeStepType timeStep)
{
  if (nullptr != inputAtTimeStep &&
      nullptr != previewImage)
  {
      mitk::GrowCutSegmentationFilter::Pointer growCutFilter = mitk::GrowCutSegmentationFilter::New();

      if (nullptr == this->GetToolManager()->GetWorkingData(0))
      {
        return;
      }

      auto *workingDataLabelSetImage = dynamic_cast<mitk::LabelSetImage *>(this->GetToolManager()->GetWorkingData(0)->GetData());

      if (nullptr == workingDataLabelSetImage)
      {
        return;
      }

      SeedImageType::Pointer seedImage = SeedImageType::New();
      workingDataLabelSetImage->SetActiveLayer(0);
      CastToItkImage(workingDataLabelSetImage, seedImage);

      growCutFilter->SetSeedImage(seedImage);
      growCutFilter->SetDistancePenalty(m_DistancePenalty);
      growCutFilter->SetInput(inputAtTimeStep);

      try
      {
        growCutFilter->Update();
      }
      catch (...)
      {
        mitkThrow() << "itkGrowCutFilter error";
      }

      auto growCutResultImage = mitk::LabelSetImage::New();
      growCutResultImage->InitializeByLabeledImage(growCutFilter->GetOutput());
      TransferLabelSetImageContent(growCutResultImage, previewImage, timeStep);
  }
}
