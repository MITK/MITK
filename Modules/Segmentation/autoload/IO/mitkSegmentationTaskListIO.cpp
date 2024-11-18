/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegmentationTaskListIO.h"
#include "mitkSegmentationIOMimeTypes.h"

#include <mitkSegmentationTaskList.h>

#include <nlohmann/json.hpp>

#include <fstream>

namespace mitk
{
  void to_json(nlohmann::json& json, const SegmentationTaskList::Task::Form& form)
  {
    json["Path"] = form.Path;
    json["Result"] = form.Result;
  }

  void from_json(const nlohmann::json& json, SegmentationTaskList::Task::Form& form)
  {
    form.Path = json["Path"].get<std::string>();
    form.Result = json["Result"].get<std::string>();
  }

  void to_json(nlohmann::json& json, const SegmentationTaskList::Task::Scene& scene)
  {
    json["Path"] = scene.Path;
    json["Image"] = scene.Image;

    if (!scene.Segmentation.empty())
      json["Segmentation"] = scene.Segmentation;
  }

  void from_json(const nlohmann::json& json, SegmentationTaskList::Task::Scene& scene)
  {
    scene.Path = json["Path"].get<std::string>();
    scene.Image = json["Image"].get<std::string>();

    auto iter = json.find("Segmentation");

    if (iter != json.end())
      scene.Segmentation = json["Segmentation"].get<std::string>();
  }

  void to_json(nlohmann::json& json, const SegmentationTaskList::Task& task)
  {
    if (task.HasName())
      json["Name"] = task.GetName();

    if (task.HasDescription())
      json["Description"] = task.GetDescription();

    if (task.HasImage())
      json["Image"] = task.GetImage().string();

    if (task.HasSegmentation())
      json["Segmentation"] = task.GetSegmentation().string();

    if (task.HasLabelName())
      json["LabelName"] = task.GetLabelName();

    if (task.HasLabelNameSuggestions())
      json["LabelNameSuggestions"] = task.GetLabelNameSuggestions().string();

    if (task.HasPreset())
      json["Preset"] = task.GetPreset().string();

    if (task.HasResult())
      json["Result"] = task.GetResult().string();

    if (task.HasDynamic())
      json["Dynamic"] = task.GetDynamic();

    if (task.HasForm())
      json["Form"] = task.GetForm();

    if (task.HasScene())
      json["Scene"] = task.GetScene();
  }

  void from_json(const nlohmann::json& json, SegmentationTaskList::Task& task)
  {
    auto iter = json.find("Name");

    if (iter != json.end())
      task.SetName(json["Name"].get<std::string>());

    iter = json.find("Description");

    if (iter != json.end())
      task.SetDescription(json["Description"].get<std::string>());

    iter = json.find("Image");

    if (iter != json.end())
      task.SetImage(json["Image"].get<std::string>());

    iter = json.find("Segmentation");

    if (iter != json.end())
      task.SetSegmentation(json["Segmentation"].get<std::string>());

    iter = json.find("LabelName");

    if (iter != json.end())
      task.SetLabelName(json["LabelName"].get<std::string>());

    iter = json.find("LabelNameSuggestions");

    if (iter != json.end())
      task.SetLabelNameSuggestions(json["LabelNameSuggestions"].get<std::string>());

    iter = json.find("Preset");

    if (iter != json.end())
      task.SetPreset(json["Preset"].get<std::string>());

    iter = json.find("Result");

    if (iter != json.end())
      task.SetResult(json["Result"].get<std::string>());

    iter = json.find("Dynamic");

    if (iter != json.end())
      task.SetDynamic(json["Dynamic"].get<bool>());

    iter = json.find("Form");

    if (iter != json.end())
      task.SetForm(json["Form"].get<SegmentationTaskList::Task::Form>());

    iter = json.find("Scene");

    if (iter != json.end())
      task.SetScene(json["Scene"].get<SegmentationTaskList::Task::Scene>());
  }
}

mitk::SegmentationTaskListIO::SegmentationTaskListIO()
  : AbstractFileIO(SegmentationTaskList::GetStaticNameOfClass(), MitkSegmentationIOMimeTypes::SEGMENTATIONTASKLIST_MIMETYPE(), "MITK Segmentation Task List")
{
  this->RegisterService();
}

std::vector<mitk::BaseData::Pointer> mitk::SegmentationTaskListIO::DoRead()
{
  auto* stream = this->GetInputStream();
  std::ifstream fileStream;

  if (nullptr == stream)
  {
    auto filename = this->GetInputLocation();

    if (filename.empty() || !fs::exists(filename))
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

  if ("MITK Segmentation Task List" != json.value("FileFormat", ""))
    mitkThrow() << "Unknown file format (expected \"MITK Segmentation Task List\")!";

  int version;

  if (version = json.value<int>("Version", 0); version < 1 || version > 3)
    mitkThrow() << "Unknown file format version (expected \"1\", \"2\", or \"3\")!";

  if (!json.contains("Tasks") || !json["Tasks"].is_array())
    mitkThrow() << "Tasks array not found!";

  auto segmentationTaskList = SegmentationTaskList::New();

  try
  {
    if (json.contains("Name"))
      segmentationTaskList->SetProperty("name", StringProperty::New(json["Name"].get<std::string>()));

    if (json.contains("Defaults"))
    {
      segmentationTaskList->SetDefaults(json["Defaults"].get<SegmentationTaskList::Task>());

      if (segmentationTaskList->GetDefaults().HasResult())
        mitkThrow() << "Defaults must not contain \"Result\"!";
    }

    for (const auto& task : json["Tasks"])
    {
      auto i = segmentationTaskList->AddTask(task.get<SegmentationTaskList::Task>());

      if (!segmentationTaskList->HasImage(i))
      {
        if (version < 3)
          mitkThrow() << "Task " << i << " must contain \"Image\"!";

        if (!segmentationTaskList->HasScene(i))
          mitkThrow() << "Task " << i << " must contain either \"Image\" or \"Scene\"!";
      }
      else
      {
        auto imagePath = this->ResolvePath(segmentationTaskList->GetImage(i));

        if (!fs::exists(imagePath))
          mitkThrow() << "Referenced image \"" << imagePath << "\" in task " << i << " does not exist!";

        if (version >= 3 && segmentationTaskList->HasScene(i))
          mitkThrow() << "Task " << i << " must not contain both \"Image\" and \"Scene\"!";
      }

      if (segmentationTaskList->HasScene(i))
      {
        auto scenePath = this->ResolvePath(segmentationTaskList->GetScene(i).Path);

        if (!fs::exists(scenePath))
          mitkThrow() << "Scene \"" << scenePath << "\" in task " << i << " does not exist!";
      }

      if (segmentationTaskList->HasForm(i))
      {
        auto formPath = this->ResolvePath(segmentationTaskList->GetForm(i).Path);

        if (!fs::exists(formPath))
          mitkThrow() << "Form \"" << formPath << "\" in task " << i << " does not exist!";
      }

      if (!segmentationTaskList->HasResult(i))
        mitkThrow() << "Task " << i << " must contain \"Result\"!";
    }
  }
  catch (const nlohmann::json::type_error& e)
  {
    mitkThrow() << e.what();
  }

  std::vector<BaseData::Pointer> result;
  result.push_back(segmentationTaskList.GetPointer());

  return result;
}

void mitk::SegmentationTaskListIO::Write()
{
  auto segmentationTaskList = dynamic_cast<const SegmentationTaskList*>(this->GetInput());

  if (nullptr == segmentationTaskList)
    mitkThrow() << "Invalid input for writing!";

  if (segmentationTaskList->GetNumberOfTasks() == 0)
    mitkThrow() << "No tasks found!";

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
    { "FileFormat", "MITK Segmentation Task List" },
    { "Version", this->GetMinimumRequiredVersion(segmentationTaskList) },
    { "Name", segmentationTaskList->GetProperty("name")->GetValueAsString() }
  };

  nlohmann::json defaults = segmentationTaskList->GetDefaults();

  if (!defaults.is_null())
    json["Defaults"] = defaults;

  nlohmann::json tasks;

  for (const auto& task : *segmentationTaskList)
    tasks.push_back(task);

  json["Tasks"] = tasks;

  *stream << std::setw(2) << json << std::endl;
}

int mitk::SegmentationTaskListIO::GetMinimumRequiredVersion(const SegmentationTaskList* segmentationTaskList) const
{
  auto hasScene = segmentationTaskList->GetDefaults().HasScene();

  if (!hasScene)
  {
    for (const auto& task : *segmentationTaskList)
    {
      if (task.HasScene())
      {
        hasScene = true;
        break;
      }
    }
  }

  if (hasScene)
    return 3;

  auto hasForm = segmentationTaskList->GetDefaults().HasForm();

  if (!hasForm)
  {
    for (const auto& task : *segmentationTaskList)
    {
      if (task.HasForm())
      {
        hasForm = true;
        break;
      }
    }
  }

  if (hasForm)
    return 2;

  return 1;
}

fs::path mitk::SegmentationTaskListIO::ResolvePath(const fs::path& path) const
{
  if (path.is_relative())
  {
    auto inputLocation = this->GetInputLocation();

    /* If we have access to properties, we are reading from an MITK scene
     * file. In this case, paths are still relative to the original input
     * location, which is preserved in the properties.
     */

    const auto* properties = this->GetProperties();

    if (properties != nullptr)
      properties->GetStringProperty("MITK.IO.reader.inputlocation", inputLocation);

    return fs::path(inputLocation).remove_filename() / path;
  }
  else
  {
    return path;
  }
}

mitk::SegmentationTaskListIO* mitk::SegmentationTaskListIO::IOClone() const
{
  return new SegmentationTaskListIO(*this);
}
