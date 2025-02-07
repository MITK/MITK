/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkOtsuTool3D.h"
#include "mitkOtsuSegmentationFilter.h"
#include <mitkLabelSetImageHelper.h>
#include <mitkImageStatisticsHolder.h>

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, OtsuTool3D, "Otsu Segmentation");
}

mitk::OtsuTool3D::OtsuTool3D()
  : SegWithPreviewTool()
{
  this->ResetsToEmptyPreviewOn();
  this->UseSpecialPreviewColorOff();
}

void mitk::OtsuTool3D::Activated()
{
  Superclass::Activated();

  m_NumberOfBins = 128;
  m_NumberOfRegions = 2;
  m_UseValley = false;
  this->SetLabelTransferScope(LabelTransferScope::AllLabels);
  this->SetLabelTransferMode(LabelTransferMode::AddLabel);
}

const char **mitk::OtsuTool3D::GetXPM() const
{
  return nullptr;
}

us::ModuleResource mitk::OtsuTool3D::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Otsu.svg");
  return resource;
}

const char* mitk::OtsuTool3D::GetName() const
{
  return "Otsu";
}

void mitk::OtsuTool3D::DoUpdatePreview(const Image* inputAtTimeStep, const Image* /*oldSegAtTimeStep*/, MultiLabelSegmentation* previewImage, TimeStepType timeStep)
{
  int numberOfThresholds = m_NumberOfRegions - 1;

  mitk::OtsuSegmentationFilter::Pointer otsuFilter = mitk::OtsuSegmentationFilter::New();
  otsuFilter->SetNumberOfThresholds(numberOfThresholds);
  otsuFilter->SetValleyEmphasis(m_UseValley);
  otsuFilter->SetNumberOfBins(m_NumberOfBins);
  otsuFilter->SetInput(inputAtTimeStep);
  otsuFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);

  try
  {
    otsuFilter->Update();
  }
  catch (...)
  {
    mitkThrow() << "itkOtsuFilter error (image dimension must be in {2, 3} and image must not be RGB)";
  }

  mitk::ImageReadAccessor newMitkImgAcc(otsuFilter->GetOutput());
  previewImage->SetVolume(newMitkImgAcc.GetData(), timeStep);
}

void mitk::OtsuTool3D::UpdatePrepare()
{
  Superclass::UpdatePrepare();
  auto preview = this->GetPreviewSegmentation();
  preview->RemoveLabels(preview->GetAllLabelValues());

  for (unsigned int i = 0; i < m_NumberOfRegions; ++i)
  {
    auto label = LabelSetImageHelper::CreateNewLabel(preview, "Otsu " + std::to_string(i), true);
    label->SetValue(i + 1);
    preview->AddLabel(label, preview->GetActiveLayer(), false, false);
  }
}

unsigned int mitk::OtsuTool3D::GetMaxNumberOfBins() const
{
  const auto min = this->GetReferenceData()->GetStatistics()->GetScalarValueMin();
  const auto max = this->GetReferenceData()->GetStatistics()->GetScalarValueMaxNoRecompute();
  return static_cast<unsigned int>(max - min) + 1;
}
