/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegmentationTaskIO.h"
#include "mitkMultilabelIOMimeTypes.h"

#include <mitkSegmentationTask.h>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

namespace mitk
{
  void to_json(nlohmann::json& json, const SegmentationTask::Subtask& subtask)
  {
    if (subtask.HasName())
      json["Name"] = subtask.GetName();

    if (subtask.HasDescription())
      json["Description"] = subtask.GetDescription();

    if (subtask.HasImage())
      json["Image"] = subtask.GetImage();

    if (subtask.HasSegmentation())
      json["Segmentation"] = subtask.GetSegmentation();

    if (subtask.HasLabelName())
      json["LabelName"] = subtask.GetLabelName();

    if (subtask.HasPreset())
      json["Preset"] = subtask.GetPreset();

    if (subtask.HasResult())
      json["Result"] = subtask.GetResult();

    if (subtask.HasDynamic())
      json["Dynamic"] = subtask.GetDynamic();
  }

  void from_json(const nlohmann::json& json, SegmentationTask::Subtask& subtask)
  {
    auto iter = json.find("Name");

    if (iter != json.end())
      subtask.SetName(json["Name"].get<std::string>());

    iter = json.find("Description");

    if (iter != json.end())
      subtask.SetDescription(json["Description"].get<std::string>());

    iter = json.find("Image");

    if (iter != json.end())
      subtask.SetImage(json["Image"].get<std::string>());

    iter = json.find("Segmentation");

    if (iter != json.end())
      subtask.SetSegmentation(json["Segmentation"].get<std::string>());

    iter = json.find("LabelName");

    if (iter != json.end())
      subtask.SetLabelName(json["LabelName"].get<std::string>());

    iter = json.find("Preset");

    if (iter != json.end())
      subtask.SetPreset(json["Preset"].get<std::string>());

    iter = json.find("Result");

    if (iter != json.end())
      subtask.SetResult(json["Result"].get<std::string>());

    iter = json.find("Dynamic");

    if (iter != json.end())
      subtask.SetDynamic(json["Dynamic"].get<bool>());
  }
}

mitk::SegmentationTaskIO::SegmentationTaskIO()
  : AbstractFileIO(SegmentationTask::GetStaticNameOfClass(), MitkMultilabelIOMimeTypes::SEGMENTATIONTASK_MIMETYPE(), "MITK Segmentation Task")
{
  this->RegisterService();
}

std::vector<mitk::BaseData::Pointer> mitk::SegmentationTaskIO::DoRead()
{
  auto* stream = this->GetInputStream();
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
  catch (const nlohmann::json::exception& e)
  {
    mitkThrow() << e.what();
  }

  if (!json.is_object())
    mitkThrow() << "Unknown file format (expected JSON object as root)!";

  if ("MITK Segmentation Task" != json.value("FileFormat", ""))
    mitkThrow() << "Unknown file format (expected \"MITK Segmentation Task\")!";

  if (1 != json.value<int>("Version", 0))
    mitkThrow() << "Unknown file format version (expected \"1\")!";

  if (!json.contains("Subtasks") || !json["Subtasks"].is_array())
    mitkThrow() << "Subtasks array not found!";

  auto segmentationTask = SegmentationTask::New();

  if (json.contains("Name"))
    segmentationTask->SetProperty("name", StringProperty::New(json["Name"].get<std::string>()));

  try
  {
    if (json.contains("Defaults"))
    {
      segmentationTask->SetDefaults(json["Defaults"].get<SegmentationTask::Subtask>());

      if (segmentationTask->GetDefaults().HasResult())
        mitkThrow() << "Defaults must not contain \"Result\"!";
    }

    for (const auto& subtask : json["Subtasks"])
    {
      auto i = segmentationTask->AddSubtask(subtask.get<SegmentationTask::Subtask>());

      if (!segmentationTask->HasImage(i))
        mitkThrow() << "Subtask " << i << " must contain \"Image\"!";

      std::filesystem::path imagePath(segmentationTask->GetImage(i));

      if (imagePath.is_relative())
      {
        auto inputLocation = this->GetInputLocation();

        /* If we have access to properties, we are reading from an MITK scene
         * file. In this case, paths are still relative to the original input
         * location, which is preserved in the properties.
         */

        const auto* properties = this->GetProperties();

        if (properties != nullptr)
          properties->GetStringProperty("MITK.IO.reader.inputlocation", inputLocation);

        imagePath = std::filesystem::path(inputLocation).remove_filename() / imagePath;
      }

      if (!std::filesystem::exists(imagePath))
        mitkThrow() << "Referenced image \"" << imagePath << "\" in subtask " << i << " does not exist!";

      if (!segmentationTask->HasResult(i))
        mitkThrow() << "Subtask " << i << " must contain \"Result\"!";
    }
  }
  catch (const nlohmann::json::type_error& e)
  {
    mitkThrow() << e.what();
  }

  std::vector<BaseData::Pointer> result;
  result.push_back(segmentationTask.GetPointer());

  return result;
}

void mitk::SegmentationTaskIO::Write()
{
  auto segmentationTask = dynamic_cast<const SegmentationTask*>(this->GetInput());

  if (nullptr == segmentationTask)
    mitkThrow() << "Invalid input for writing!";

  if (segmentationTask->GetNumberOfSubtasks() == 0)
    mitkThrow() << "No subtasks found!";

  auto* stream = this->GetOutputStream();
  std::ofstream fileStream;

  if (nullptr == stream)
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

  nlohmann::ordered_json json = {
    { "FileFormat", "MITK Segmentation Task" },
    { "Version", 1 },
    { "Name", segmentationTask->GetProperty("name")->GetValueAsString() }
  };

  nlohmann::json defaults = segmentationTask->GetDefaults();

  if (!defaults.is_null())
    json["Defaults"] = defaults;

  nlohmann::json subtasks;

  for (const auto& subtask : *segmentationTask)
    subtasks.push_back(subtask);

  json["Subtasks"] = subtasks;

  *stream << std::setw(2) << json << std::endl;
}

mitk::SegmentationTaskIO* mitk::SegmentationTaskIO::IOClone() const
{
  return new SegmentationTaskIO(*this);
}
