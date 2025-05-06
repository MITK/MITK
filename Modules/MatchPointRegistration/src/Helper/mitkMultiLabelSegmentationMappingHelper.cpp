/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMultiLabelSegmentationMappingHelper.h"


#include <mitkGeometry3D.h>

#include "mapRegistration.h"

#include "mitkImageMappingHelper.h"
#include "mitkRegistrationHelper.h"



mitk::MultiLabelSegmentation::Pointer
  mitk::MultiLabelSegmentationMappingHelper::map(const MultiLabelSegmentation* input, const RegistrationType* registration,
  bool throwOnOutOfInputAreaError, const ResultGeometryType* resultGeometry,
  bool throwOnMappingError, const MultiLabelSegmentation::LabelValueType& errorValue)
{
  if (!registration)
  {
    mitkThrow() << "Cannot map image. Passed registration wrapper pointer is nullptr.";
  }
  if (!input)
  {
    mitkThrow() << "Cannot map image. Passed segmentation pointer is nullptr.";
  }

  auto resultLabelSetImage = MultiLabelSegmentation::New();

  auto mappedTimeGeometry = ImageMappingHelper::CreateResultTimeGeometry(input, resultGeometry);

  auto resultTemplate = mitk::Image::New();
  resultTemplate->Initialize(MultiLabelSegmentation::GetPixelType(), *mappedTimeGeometry, 1, input->GetTimeSteps());

  resultLabelSetImage->Initialize(resultTemplate, true, false);

  for (MultiLabelSegmentation::GroupIndexType groupID = 0; groupID < input->GetNumberOfGroups(); ++groupID)
  {
    auto inputGroupImage = input->GetGroupImage(groupID);
    auto mappedGroupImage = ImageMappingHelper::map(inputGroupImage, registration, throwOnOutOfInputAreaError, 0, resultGeometry, throwOnMappingError, errorValue, mitk::ImageMappingInterpolator::NearestNeighbor);

    resultLabelSetImage->AddGroup(mappedGroupImage, input->GetConstLabelsByValue(input->GetLabelValuesByGroup(groupID)));
  }

  resultLabelSetImage->SetActiveLabel(input->GetActiveLabel()->GetValue());

  return resultLabelSetImage;
}

mitk::MultiLabelSegmentation::Pointer
  mitk::MultiLabelSegmentationMappingHelper::map(const MultiLabelSegmentation* input, const MITKRegistrationType* registration,
  bool throwOnOutOfInputAreaError, const ResultGeometryType* resultGeometry,
  bool throwOnMappingError, const MultiLabelSegmentation::LabelValueType& errorValue)
{
  if (!registration)
  {
    mitkThrow() << "Cannot map image. Passed registration wrapper pointer is nullptr.";
  }
  if (!registration->GetRegistration())
  {
    mitkThrow() << "Cannot map image. Passed registration wrapper contains no registration.";
  }
  if (!input)
  {
    mitkThrow() << "Cannot map image. Passed segmentation pointer is nullptr.";
  }

  return map(input, registration->GetRegistration(), throwOnOutOfInputAreaError, resultGeometry, throwOnMappingError, errorValue);
}

