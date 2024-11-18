/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegmentationIOMimeTypes.h"

#include <mitkFileSystem.h>
#include <mitkIOMimeTypes.h>

#include <fstream>

#include <nlohmann/json.hpp>

mitk::MitkSegmentationIOMimeTypes::MitkSegmentationTaskListMimeType::MitkSegmentationTaskListMimeType()
  : CustomMimeType(SEGMENTATIONTASKLIST_MIMETYPE_NAME())
{
  this->AddExtension("json");
  this->SetCategory("MITK Segmentation Task List");
  this->SetComment("MITK Segmentation Task List");
}

bool mitk::MitkSegmentationIOMimeTypes::MitkSegmentationTaskListMimeType::AppliesTo(const std::string& path) const
{
  bool result = CustomMimeType::AppliesTo(path);

  if (!fs::exists(path)) // T18572
    return result;

  std::ifstream file(path);

  if (!file.is_open())
    return false;

  auto json = nlohmann::json::parse(file, nullptr, false);

  if (json.is_discarded() || !json.is_object())
    return false;

  if ("MITK Segmentation Task List" != json.value("FileFormat", ""))
    return false;

  if (auto version = json.value<int>("Version", 0); version < 1 || version > 3)
    return false;

  return true;
}

mitk::MitkSegmentationIOMimeTypes::MitkSegmentationTaskListMimeType* mitk::MitkSegmentationIOMimeTypes::MitkSegmentationTaskListMimeType::Clone() const
{
  return new MitkSegmentationTaskListMimeType(*this);
}

mitk::MitkSegmentationIOMimeTypes::MitkSegmentationTaskListMimeType mitk::MitkSegmentationIOMimeTypes::SEGMENTATIONTASKLIST_MIMETYPE()
{
  return MitkSegmentationTaskListMimeType();
}

std::string mitk::MitkSegmentationIOMimeTypes::SEGMENTATIONTASKLIST_MIMETYPE_NAME()
{
  return IOMimeTypes::DEFAULT_BASE_NAME() + ".segmentationtasklist";
}

std::vector<mitk::CustomMimeType*> mitk::MitkSegmentationIOMimeTypes::Get()
{
  std::vector<CustomMimeType*> mimeTypes;
  mimeTypes.push_back(SEGMENTATIONTASKLIST_MIMETYPE().Clone());
  return mimeTypes;
}
