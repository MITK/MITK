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

mitk::MitkMultilabelIOMimeTypes::MitkSegmentationTaskListMimeType::MitkSegmentationTaskListMimeType()
  : CustomMimeType(SEGMENTATIONTASKLIST_MIMETYPE_NAME())
{
  this->AddExtension("json");
  this->SetCategory("MITK Segmentation Task List");
  this->SetComment("MITK Segmentation Task List");
}

bool mitk::MitkMultilabelIOMimeTypes::MitkSegmentationTaskListMimeType::AppliesTo(const std::string& path) const
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

  if ("MITK Segmentation Task List" != json.value("FileFormat", ""))
    return false;

  if (1 != json.value<int>("Version", 0))
    return false;

  return true;
}

mitk::MitkMultilabelIOMimeTypes::MitkSegmentationTaskListMimeType* mitk::MitkMultilabelIOMimeTypes::MitkSegmentationTaskListMimeType::Clone() const
{
  return new MitkSegmentationTaskListMimeType(*this);
}

mitk::MitkMultilabelIOMimeTypes::MitkSegmentationTaskListMimeType mitk::MitkMultilabelIOMimeTypes::SEGMENTATIONTASKLIST_MIMETYPE()
{
  return MitkSegmentationTaskListMimeType();
}

std::string mitk::MitkMultilabelIOMimeTypes::SEGMENTATIONTASKLIST_MIMETYPE_NAME()
{
  return IOMimeTypes::DEFAULT_BASE_NAME() + ".segmentationtasklist";
}

std::vector<mitk::CustomMimeType*> mitk::MitkMultilabelIOMimeTypes::Get()
{
  std::vector<CustomMimeType*> mimeTypes;
  mimeTypes.push_back(SEGMENTATIONTASKLIST_MIMETYPE().Clone());
  return mimeTypes;
}
