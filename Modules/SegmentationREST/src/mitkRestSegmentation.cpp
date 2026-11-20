/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestSegmentation.h>

#include <mitkErrorResponse.h>
#include <mitkIPreferences.h>
#include <mitkIRestServerService.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkLabelSuggestionHelper.h>
#include <mitkRestHelpers.h>
#include <mitkRestPoints.h>

#include <QFileInfo>
#include <QString>

#include <algorithm>
#include <cstdint>
#include <limits>

nlohmann::json mitk::DescribeRestNode(DataNode* node, const IRestServerService& server)
{
  if (node == nullptr)
    return nullptr;

  return { { "uid", server.GetNodeUid(node) }, { "name", node->GetName() } };
}

mitk::Color mitk::ParseRestColorOrThrow(const nlohmann::json& json, const std::string& path)
{
  const auto color = ParseRestColor(json.is_string() ? json.get<std::string>() : std::string());

  if (!color.has_value())
    throw RestException(ErrorResponse::InvalidRequest("A color must be written like \"#ff8800\".", path));

  return *color;
}

bool mitk::IsRestMeasureRequested(const RestRequest& request)
{
  const auto value = request.GetQueryParam("measure");

  if (!value.has_value() || *value == "false")
    return false;

  if (*value == "true")
    return true;

  throw RestException(ErrorResponse::InvalidRequest("\"measure\" must be true or false.", request.Path));
}

nlohmann::json mitk::DescribeRestLabel(const Label* label, MultiLabelSegmentation::GroupIndexType group)
{
  return {
    { "value", label->GetValue() },
    { "name", label->GetName() },
    { "color", ToRestColor(label->GetColor()) },
    { "visible", label->GetVisible() },
    { "locked", label->GetLocked() },
    { "group", group }
  };
}

nlohmann::json mitk::DescribeRestLabels(MultiLabelSegmentation* segmentation, const std::optional<RestTimeSelection>& measureAt)
{
  auto labels = nlohmann::json::array();
  const auto timeSteps = measureAt.has_value() ? measureAt->GetTimeSteps(segmentation) : std::vector<TimeStepType>();

  for (MultiLabelSegmentation::GroupIndexType group = 0; group < segmentation->GetNumberOfGroups(); ++group)
  {
    const auto* groupImage = segmentation->GetGroupImage(group);
    const auto spacing = groupImage->GetGeometry()->GetSpacing();
    const auto voxelMilliliters = spacing[0] * spacing[1] * spacing[2] / 1000.0;

    // Nothing for a time step at which a tool writes into the segmentation: the measure never waits for it.
    std::vector<std::optional<MultiLabelSegmentation::LabelMeasureMapType>> measures;

    for (const auto timeStep : timeSteps)
    {
      try
      {
        measures.emplace_back(segmentation->MeasureLabels(group, timeStep));
      }
      catch (const Exception&)
      {
        measures.emplace_back();
      }
    }

    const auto addMeasure = [&](nlohmann::json& json, std::size_t i, MultiLabelSegmentation::LabelValueType value) {
      if (measures.size() <= i || !measures[i].has_value())
      {
        json["voxels"] = nullptr;
        return;
      }

      const auto found = measures[i]->find(value);
      const std::uint64_t count = found != measures[i]->end() ? found->second.VoxelCount : 0;
      json["voxels"] = count;
      json["volume_ml"] = count * voxelMilliliters;

      if (found == measures[i]->end())
      {
        json["center"] = nullptr;
        return;
      }

      Point3D center;
      groupImage->GetGeometry(timeSteps[i])->IndexToWorld(found->second.CentroidIndex, center);
      json["center"] = ToRestJson(center);
    };

    for (const auto value : segmentation->GetLabelValuesByGroup(group))
    {
      const auto label = segmentation->GetLabel(value);

      if (label.IsNull())
        continue;

      auto json = DescribeRestLabel(label, group);

      if (measureAt.has_value() && measureAt->All)
      {
        auto steps = nlohmann::json::array();

        for (std::size_t i = 0; i < timeSteps.size(); ++i)
        {
          auto step = DescribeRestTimeStep(segmentation, timeSteps[i]);
          addMeasure(step, i, value);
          steps.push_back(std::move(step));
        }

        json["time_steps"] = std::move(steps);
      }
      else if (measureAt.has_value())
      {
        addMeasure(json, 0, value);
      }

      labels.push_back(std::move(json));
    }
  }

  return labels;
}

nlohmann::json mitk::DescribeRestGroup(MultiLabelSegmentation* segmentation, MultiLabelSegmentation::GroupIndexType group)
{
  auto labels = nlohmann::json::array();

  for (const auto value : segmentation->GetLabelValuesByGroup(group))
  {
    if (const auto label = segmentation->GetLabel(value); label.IsNotNull())
      labels.push_back(DescribeRestLabel(label, group));
  }

  return {
    { "index", group },
    { "name", segmentation->GetGroupName(group) },
    { "display_name", LabelSetImageHelper::CreateDisplayGroupName(segmentation, group) },
    { "labels", labels }
  };
}

nlohmann::json mitk::DescribeRestGroups(MultiLabelSegmentation* segmentation)
{
  auto groups = nlohmann::json::array();

  for (MultiLabelSegmentation::GroupIndexType group = 0; group < segmentation->GetNumberOfGroups(); ++group)
    groups.push_back(DescribeRestGroup(segmentation, group));

  return groups;
}

nlohmann::json mitk::DescribeRestRestrictions(const IPreferences& preferences)
{
  const auto preset = QFileInfo(QString::fromStdString(preferences.Get("label set preset", ""))).fileName().toStdString();

  return {
    { "label_set_preset", preset.empty() ? nlohmann::json() : nlohmann::json(preset) },
    { "label_suggestions_enforced", LabelSuggestionHelper::GetSuggestionPreferences().enforceSuggestions }
  };
}

nlohmann::json mitk::DescribeRestLabelSuggestions()
{
  const auto suggestionPreferences = LabelSuggestionHelper::GetSuggestionPreferences();

  // Loaded as the view loads them for its label inspector.
  auto helper = LabelSuggestionHelper::New();
  helper->LoadStandardSuggestions();

  if (!suggestionPreferences.externalLabelSuggestionFile.empty())
  {
    try
    {
      helper->ParseSuggestions(suggestionPreferences.externalLabelSuggestionFile, suggestionPreferences.replaceStandardSuggestions);
    }
    catch (const Exception& e)
    {
      MITK_WARN << "The label suggestions of the preferences cannot be read: " << e.GetDescription();
    }
  }

  auto names = nlohmann::json::array();

  for (const auto& suggestion : helper->GetAllSuggestions())
    names.push_back(suggestion->GetName());

  return {
    { "names", names },
    { "enforced", suggestionPreferences.enforceSuggestions }
  };
}

void mitk::ThrowIfUnknownRestField(const nlohmann::json& body, const std::vector<std::string>& fields, const std::string& subject, const std::string& path)
{
  if (!body.is_object())
    throw RestException(ErrorResponse::InvalidRequest("The body must be an object with the fields to change.", path));

  for (const auto& item : body.items())
  {
    if (std::find(fields.begin(), fields.end(), item.key()) != fields.end())
      continue;

    std::string list;

    for (std::size_t i = 0; i < fields.size(); ++i)
      list += (i == 0 ? "" : i + 1 == fields.size() ? " and " : ", ") + fields[i];

    throw RestException(ErrorResponse::InvalidRequest("\"" + item.key() + "\" is no field of " + subject + ". " +
      (fields.size() == 1 ? "The field is " : "The fields are ") + list + ".", path));
  }
}

mitk::Label::Pointer mitk::GetRestLabel(MultiLabelSegmentation* segmentation, const RestRequest& request)
{
  const auto value = ParseRestNumber<MultiLabelSegmentation::LabelValueType>(request.PathParams.at("value"));

  if (!value.has_value())
  {
    throw RestException(ErrorResponse::InvalidRequest("The label value must be a whole number of at most " +
      std::to_string(std::numeric_limits<MultiLabelSegmentation::LabelValueType>::max()) + ".", request.Path));
  }

  auto label = segmentation->GetLabel(*value);

  if (label.IsNull())
    ThrowRestError("LABEL_NOT_FOUND", "Not Found", "The segmentation has no label " + std::to_string(*value) + ".", 404, request.Path);

  return label;
}

mitk::MultiLabelSegmentation::GroupIndexType mitk::GetRestGroupIndex(const MultiLabelSegmentation* segmentation, const RestRequest& request)
{
  const auto group = ParseRestNumber<MultiLabelSegmentation::GroupIndexType>(request.PathParams.at("index"));

  if (!group.has_value())
    throw RestException(ErrorResponse::InvalidRequest("The group index must be a whole number.", request.Path));

  if (!segmentation->ExistGroup(*group))
    ThrowRestError("GROUP_NOT_FOUND", "Not Found", "The segmentation has no group " + std::to_string(*group) + ".", 404, request.Path);

  return *group;
}
