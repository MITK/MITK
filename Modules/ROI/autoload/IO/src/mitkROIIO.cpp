/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkROIIO.h"
#include <mitkROI.h>
#include <mitkROIIOMimeTypes.h>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

mitk::ROIIO::ROIIO()
  : AbstractFileIO(ROI::GetStaticNameOfClass(), MitkROIIOMimeTypes::ROI_MIMETYPE(), "MITK ROI")
{
  this->RegisterService();
}

std::vector<mitk::BaseData::Pointer> mitk::ROIIO::DoRead()
{
  auto *stream = this->GetInputStream();
  std::ifstream fileStream;

  if (nullptr == stream)
  {
    auto filename = this->GetInputLocation();

    if (filename.empty() || !std::filesystem::exists(filename))
      mitkThrow() << "Invalid or nonexistent filename: \"" << filename << "\"!";

    fileStream.open(filename);

    if (!fileStream.is_open())
      mitkThrow() << "Could not open file \"" << filename << "\" for reading!";

    stream = &fileStream;
  }

  nlohmann::json json;

  try
  {
    json = nlohmann::json::parse(*stream);
  }
  catch (const nlohmann::json::exception &e)
  {
    mitkThrow() << e.what();
  }

  if (!json.is_object())
    mitkThrow() << "Unknown file format (expected JSON object as root)!";

  if ("MITK ROI" != json.value("FileFormat", ""))
    mitkThrow() << "Unknown file format (expected \"MITK ROI\")!";

  if (1 != json.value<int>("Version", 0))
    mitkThrow() << "Unknown file format version (expected \"1\")!";

  auto geometry = Geometry3D::New();

  if (json.contains("Geometry"))
  {
    auto jsonGeometry = json["Geometry"];

    if (!jsonGeometry.is_object())
      mitkThrow() << "Geometry is expected to be a JSON object.";

    auto geometryType = jsonGeometry.value<std::string>("Type", "Embedded");

    if (geometryType != "Embedded")
      mitkThrow() << "Unknown geometry type \"" << geometryType << "\"!";

    if (jsonGeometry.contains("Origin"))
      geometry->SetOrigin(jsonGeometry["Origin"].get<Point3D>());

    if (jsonGeometry.contains("Spacing"))
      geometry->SetSpacing(jsonGeometry["Spacing"].get<Vector3D>());
  }

  if (!json.contains("ROIs") || !json["ROIs"].is_array())
    mitkThrow() << "ROIs array not found!";

  std::vector<BaseData::Pointer> results;

  try
  {
    for (const auto& roi : json["ROIs"])
    {
      auto result = ROI::New();
      result->SetGeometry(geometry);
      results.push_back(result.GetPointer());
    }
  }
  catch (const nlohmann::json::type_error &e)
  {
    mitkThrow() << e.what();
  }

  return results;
}

void mitk::ROIIO::Write()
{
}

mitk::ROIIO* mitk::ROIIO::IOClone() const
{
  return new ROIIO(*this);
}
