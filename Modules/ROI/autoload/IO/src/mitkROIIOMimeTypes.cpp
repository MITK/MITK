/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkROIIOMimeTypes.h>
#include <mitkIOMimeTypes.h>

#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

mitk::MitkROIIOMimeTypes::MitkROIMimeType::MitkROIMimeType()
  : CustomMimeType(ROI_MIMETYPE_NAME())
{
  this->AddExtension("json");
  this->SetCategory("MITK ROI");
  this->SetComment("MITK ROI");
}

bool mitk::MitkROIIOMimeTypes::MitkROIMimeType::AppliesTo(const std::string& path) const
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

  if ("MITK ROI" != json.value("FileFormat", ""))
    return false;

  if (1 != json.value<int>("Version", 0))
    return false;

  return true;
}

mitk::MitkROIIOMimeTypes::MitkROIMimeType* mitk::MitkROIIOMimeTypes::MitkROIMimeType::Clone() const
{
  return new MitkROIMimeType(*this);
}

mitk::MitkROIIOMimeTypes::MitkROIMimeType mitk::MitkROIIOMimeTypes::ROI_MIMETYPE()
{
  return MitkROIMimeType();
}

std::string mitk::MitkROIIOMimeTypes::ROI_MIMETYPE_NAME()
{
  return IOMimeTypes::DEFAULT_BASE_NAME() + ".roi";
}

std::vector<mitk::CustomMimeType*> mitk::MitkROIIOMimeTypes::Get()
{
  std::vector<CustomMimeType*> mimeTypes;
  mimeTypes.push_back(ROI_MIMETYPE().Clone());
  return mimeTypes;
}
