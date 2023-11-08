/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkROIIO.h"
#include <mitkProportionalTimeGeometry.h>
#include <mitkROI.h>
#include <mitkROIIOMimeTypes.h>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

namespace
{
  mitk::TimeGeometry::Pointer ReadGeometry(const nlohmann::json& jsonGeometry)
  {
    auto geometry = mitk::Geometry3D::New();
    geometry->ImageGeometryOn();

    if (!jsonGeometry.is_object())
      mitkThrow() << "Geometry is expected to be a JSON object.";

    if (jsonGeometry.contains("Origin"))
      geometry->SetOrigin(jsonGeometry["Origin"].get<mitk::Point3D>());

    if (jsonGeometry.contains("Spacing"))
      geometry->SetSpacing(jsonGeometry["Spacing"].get<mitk::Vector3D>());

    if (jsonGeometry.contains("Size"))
    {
      auto size = jsonGeometry["Size"].get<mitk::Vector3D>();
      mitk::BaseGeometry::BoundsArrayType bounds({ 0.0, size[0], 0.0, size[1], 0.0, size[2] });
      geometry->SetBounds(bounds);
    }

    auto timeSteps = jsonGeometry.contains("TimeSteps")
      ? jsonGeometry["TimeSteps"].get<mitk::TimeStepType>()
      : 1;

    auto result = mitk::ProportionalTimeGeometry::New();
    result->Initialize(geometry, timeSteps);

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

    result->SetTimeGeometry(ReadGeometry(json["Geometry"]));

    if (json.contains("Properties"))
    {
      auto properties = mitk::PropertyList::New();
      properties->FromJSON(json["Properties"]);
      result->GetPropertyList()->ConcatenatePropertyList(properties);
    }

    for (const auto& jsonROI : json["ROIs"])
    {
      ROI::Element roi(jsonROI["ID"].get<unsigned int>());

      if (jsonROI.contains("TimeSteps"))
      {
        for (const auto& jsonTimeStep : jsonROI["TimeSteps"])
        {
          auto t = jsonTimeStep["t"].get<TimeStepType>();

          roi.SetMin(jsonTimeStep["Min"].get<Point3D>(), t);
          roi.SetMax(jsonTimeStep["Max"].get<Point3D>(), t);

          if (jsonTimeStep.contains("Properties"))
          {
            auto properties = mitk::PropertyList::New();
            properties->FromJSON(jsonTimeStep["Properties"]);
            roi.SetProperties(properties, t);
          }
        }
      }
      else
      {
        roi.SetMin(jsonROI["Min"].get<Point3D>());
        roi.SetMax(jsonROI["Max"].get<Point3D>());
      }

      if (jsonROI.contains("Properties"))
      {
        auto properties = mitk::PropertyList::New();
        properties->FromJSON(jsonROI["Properties"]);
        roi.SetDefaultProperties(properties);
      }

      result->AddElement(roi);
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
