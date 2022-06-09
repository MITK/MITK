/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMultilabelIOMimeTypes.h"
#include <mitkIOMimeTypes.h>

#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

mitk::MitkMultilabelIOMimeTypes::MitkSegmentationTaskMimeType::MitkSegmentationTaskMimeType()
  : CustomMimeType(SEGMENTATIONTASK_MIMETYPE_NAME())
{
  this->AddExtension("json");
  this->SetCategory("MITK Segmentation Task");
  this->SetComment("MITK Segmentation Task");
}

bool mitk::MitkMultilabelIOMimeTypes::MitkSegmentationTaskMimeType::AppliesTo(const std::string& path) const
{
  bool result = CustomMimeType::AppliesTo(path);

  if (!std::filesystem::exists(path)) // T18572
    return result;

  std::ifstream file(path);

  if (!file.is_open())
    return false;

  auto json = nlohmann::json::parse(file, nullptr, false);

  if (json.is_discarded() || !json.is_object())
    return false;

  if ("MITK Segmentation Task" != json.value("FileFormat", ""))
    return false;

  if (1 != json.value<int>("Version", 0))
    return false;

  return true;
}

mitk::MitkMultilabelIOMimeTypes::MitkSegmentationTaskMimeType* mitk::MitkMultilabelIOMimeTypes::MitkSegmentationTaskMimeType::Clone() const
{
  return new MitkSegmentationTaskMimeType(*this);
}

mitk::MitkMultilabelIOMimeTypes::MitkSegmentationTaskMimeType mitk::MitkMultilabelIOMimeTypes::SEGMENTATIONTASK_MIMETYPE()
{
  return MitkSegmentationTaskMimeType();
}

std::string mitk::MitkMultilabelIOMimeTypes::SEGMENTATIONTASK_MIMETYPE_NAME()
{
  return IOMimeTypes::DEFAULT_BASE_NAME() + ".segmentationtask";
}

std::vector<mitk::CustomMimeType*> mitk::MitkMultilabelIOMimeTypes::Get()
{
  std::vector<CustomMimeType*> mimeTypes;
  mimeTypes.push_back(SEGMENTATIONTASK_MIMETYPE().Clone());
  return mimeTypes;
}
