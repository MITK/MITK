/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegmentationHelper.h"

#include <mitkImageTimeSelector.h>
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>

mitk::Image::Pointer mitk::SegmentationHelper::GetStaticSegmentationTemplate(const Image* referenceImage)
{
  if (referenceImage == nullptr)
    return nullptr;

  const auto* referenceGeometry = referenceImage->GetTimeGeometry();

  const auto timePoint = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
  const auto timeStep = referenceGeometry->IsValidTimePoint(timePoint)
    ? referenceGeometry->TimePointToTimeStep(timePoint)
    : 0;

  auto geometry = ProportionalTimeGeometry::New();
  geometry->SetFirstTimePoint(referenceGeometry->GetMinimumTimePoint());
  geometry->SetStepDuration(referenceGeometry->GetMaximumTimePoint() - referenceGeometry->GetMinimumTimePoint());
  geometry->SetTimeStepGeometry(referenceImage->GetGeometry(timeStep), 0);

  auto selector = mitk::ImageTimeSelector::New();
  selector->SetInput(referenceImage);
  selector->SetTimeNr(timeStep);
  selector->Update();

  Image::Pointer templateImage = selector->GetOutput();
  templateImage->SetTimeGeometry(geometry);

  return templateImage;
}
