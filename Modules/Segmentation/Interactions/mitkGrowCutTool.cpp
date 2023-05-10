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

mitk::GrowCutTool::GrowCutTool() : SegWithPreviewTool(true, "PressMoveReleaseAndPointSetting")
{
  this->ResetsToEmptyPreviewOn();
  this->UseSpecialPreviewColorOff();
}

mitk::GrowCutTool::~GrowCutTool() {}

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

bool mitk::GrowCutTool::SeedImageIsValid()
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

  auto numberOfLabels = workingDataLabelSetImage->GetNumberOfLabels(workingDataLabelSetImage->GetActiveLayer());

  if (numberOfLabels >= 2)
  {
    return true;
  }

  return false;
}

void mitk::GrowCutTool::DoUpdatePreview(const Image *inputAtTimeStep,
                                        const Image * oldSegAtTimeStep,
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

      SeedImageType::Pointer seedImage = SeedImageType::New();
      CastToItkImage(oldSegAtTimeStep, seedImage);

      growCutFilter->SetSeedImage(seedImage);
      growCutFilter->SetDistancePenalty(m_DistancePenalty);
      growCutFilter->SetInput(inputAtTimeStep);
      growCutFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);

      try
      {
        growCutFilter->Update();
      }
      catch (...)
      {
        mitkThrow() << "itkGrowCutFilter error";
      }

      auto growCutResultImage = growCutFilter->GetOutput();

      mitk::ImageReadAccessor newMitkImgAcc(growCutResultImage);
      previewImage->SetVolume(newMitkImgAcc.GetData(), timeStep);
  }
}
