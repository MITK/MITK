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

#include <filesystem>
#include <fstream>

namespace
{
  int CheckFileFormat(const nlohmann::json& json)
  {
    if ("MITK ROI" != json["FileFormat"].get<std::string>())
      mitkThrow() << "Unknown file format (expected \"MITK ROI\")!";

    auto version = json["Version"].get<int>();

    if (1 != version)
      mitkThrow() << "Unknown file format version (expected version 1)!";

    return version;
  }

  mitk::Vector3D GetSize(const mitk::BaseGeometry* geometry)
  {
    auto bounds = geometry->GetBounds();

    mitk::Vector3D result;
    result[0] = bounds[1];
    result[1] = bounds[3];
    result[2] = bounds[5];

    return result;
  }

  void SetSize(mitk::BaseGeometry* geometry, const mitk::Vector3D& size)
  {
    mitk::BaseGeometry::BoundsArrayType bounds({ 0.0, size[0], 0.0, size[1], 0.0, size[2] });
    geometry->SetBounds(bounds);
  }

  mitk::TimeGeometry::Pointer ReadGeometry(const nlohmann::json& jGeometry)
  {
    auto geometry = mitk::Geometry3D::New();
    geometry->ImageGeometryOn();

    if (!jGeometry.is_object())
      mitkThrow() << "Geometry is expected to be a JSON object.";

    if (jGeometry.contains("Origin"))
      geometry->SetOrigin(jGeometry["Origin"].get<mitk::Point3D>());

    if (jGeometry.contains("Spacing"))
      geometry->SetSpacing(jGeometry["Spacing"].get<mitk::Vector3D>());

    if (jGeometry.contains("Size"))
      SetSize(geometry, jGeometry["Size"].get<mitk::Vector3D>());

    auto timeSteps = jGeometry.contains("TimeSteps")
      ? jGeometry["TimeSteps"].get<mitk::TimeStepType>()
      : 1;

    auto result = mitk::ProportionalTimeGeometry::New();
    result->Initialize(geometry, timeSteps);

    return result;
  }

  nlohmann::json WriteGeometry(const mitk::TimeGeometry* timeGeometry)
  {
    auto geometry = timeGeometry->GetGeometryForTimeStep(0);

    nlohmann::json result = {
      { "Origin", geometry->GetOrigin() },
      { "Spacing", geometry->GetSpacing() },
      { "Size", GetSize(geometry) }
    };

    auto timeSteps = timeGeometry->CountTimeSteps();

    if (timeSteps > 1)
      result["TimeSteps"] = timeSteps;

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
    auto j = nlohmann::json::parse(*stream);

    CheckFileFormat(j);

    auto geometry = ReadGeometry(j["Geometry"]);
    result->SetTimeGeometry(geometry);

    if (j.contains("Name"))
      result->SetProperty("name", mitk::StringProperty::New(j["Name"].get<std::string>()));

    if (j.contains("Caption"))
      result->SetProperty("caption", mitk::StringProperty::New(j["Caption"].get<std::string>()));

    for (const auto& roi : j["ROIs"])
      result->AddElement(roi.get<ROI::Element>());
  }
  catch (const nlohmann::json::exception &e)
  {
    mitkThrow() << e.what();
  }

  return { result };
}

void mitk::ROIIO::Write()
{
  auto input = dynamic_cast<const ROI*>(this->GetInput());

  if (input == nullptr)
    mitkThrow() << "Invalid input for writing!";

  if (input->GetNumberOfElements() == 0)
    mitkThrow() << "No ROIs found!";

  auto* stream = this->GetOutputStream();
  std::ofstream fileStream;

  if (stream == nullptr)
  {
    auto filename = this->GetOutputLocation();

    if (filename.empty())
      mitkThrow() << "Neither an output stream nor an output filename was specified!";

    fileStream.open(filename);

    if (!fileStream.is_open())
      mitkThrow() << "Could not open file \"" << filename << "\" for writing!";

    stream = &fileStream;
  }

  if (!stream->good())
    mitkThrow() << "Stream for writing is not good!";

  nlohmann::ordered_json j = {
    { "FileFormat", "MITK ROI" },
    { "Version", 1 }
  };

  if (auto name = input->GetProperty("name"); name.IsNotNull())
    j["Name"] = name->GetValueAsString();

  j["Geometry"] = WriteGeometry(input->GetTimeGeometry());

  auto caption = input->GetConstProperty("caption");

  if (caption.IsNotNull())
    j["Caption"] = caption->GetValueAsString();

  nlohmann::json rois;

  for (const auto& roi : *input)
    rois.push_back(roi.second);

  j["ROIs"] = rois;

  *stream << std::setw(2) << j << std::endl;
}

mitk::ROIIO* mitk::ROIIO::IOClone() const
{
  return new ROIIO(*this);
}
