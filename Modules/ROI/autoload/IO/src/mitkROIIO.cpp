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

namespace
{
  mitk::Geometry3D::Pointer ReadGeometry(const nlohmann::json& jsonGeometry)
  {
    auto result = mitk::Geometry3D::New();
    result->ImageGeometryOn();

    if (!jsonGeometry.is_object())
      mitkThrow() << "Geometry is expected to be a JSON object.";

    if (jsonGeometry.contains("Origin"))
      result->SetOrigin(jsonGeometry["Origin"].get<mitk::Point3D>());

    if (jsonGeometry.contains("Spacing"))
      result->SetSpacing(jsonGeometry["Spacing"].get<mitk::Vector3D>());

    if (jsonGeometry.contains("Size"))
    {
      auto size = jsonGeometry["Size"].get<mitk::Vector3D>();
      mitk::BaseGeometry::BoundsArrayType bounds({ 0.0, size[0], 0.0, size[1], 0.0, size[2] });
      result->SetBounds(bounds);
    }

    return result;
  }
}

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

  auto result = ROI::New();

  try
  {
    auto json = nlohmann::json::parse(*stream);

    if ("MITK ROI" != json["FileFormat"].get<std::string>())
      mitkThrow() << "Unknown file format (expected \"MITK ROI\")!";

    if (1 != json["Version"].get<int>())
      mitkThrow() << "Unknown file format version (expected version 1)!";

    result->SetGeometry(ReadGeometry(json["Geometry"]));

    for (const auto& jsonROI : json["ROIs"])
    {
    }
  }
  catch (const nlohmann::json::exception &e)
  {
    mitkThrow() << e.what();
  }

  return { result };
}

void mitk::ROIIO::Write()
{
}

mitk::ROIIO* mitk::ROIIO::IOClone() const
{
  return new ROIIO(*this);
}
