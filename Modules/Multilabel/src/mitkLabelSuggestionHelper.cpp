/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLabelSuggestionHelper.h"

#include <algorithm>

#include <mitkDICOMSegmentationConstants.h>
#include <mitkDICOMSegmentationPropertyHelper.h>

#include <mitkMultiLabelIOHelper.h>
#include <mitkProperties.h>
#include <mitkStringProperty.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include "usModuleResource.h"
#include "usModuleResourceStream.h"

namespace
{
  const constexpr int MULTILABEL_SEGMENTATION_VERSION_VALUE = 4;

  std::optional<unsigned int> GetMaxInstanceOccurrenceOfLabel(const mitk::Label* label, bool globalOnlyOnce)
  {
    auto property = label->GetConstProperty(mitk::LabelSuggestionHelper::PROPERTY_MAX_INSTANCE_OCCURRENCE);

    if (nullptr == property)
      return globalOnlyOnce ? std::optional{ 1u } : std::nullopt; // Unlimited

    auto intProperty = dynamic_cast<const mitk::IntProperty*>(property.GetPointer());
    if (nullptr == intProperty)
      return globalOnlyOnce ? std::optional{ 1u } : std::nullopt; // Unlimited

    return intProperty->GetValue();
  }

  void MergeLabels(mitk::LabelSuggestionHelper::ConstLabelVectorType& target, const mitk::LabelSuggestionHelper::ConstLabelVectorType& source)
  {
    for (const auto& label : source)
    {
      const auto labelName = label->GetName();
      bool exists =
        std::any_of(target.begin(), target.end(), [&labelName](const mitk::Label* l) { return l->GetName() == labelName; });
      if (!exists)
        target.push_back(label);
    }
  }
}

namespace mitk
{

  LabelSuggestionHelper::LabelSuggestionHelper() {}

  LabelSuggestionHelper::~LabelSuggestionHelper() {}

  void LabelSuggestionHelper::LoadStandardSuggestions()
  {
    auto suggestionPref = mitk::LabelSuggestionHelper::GetSuggestionPreferences();

    auto json = GetStandardSuggesitions();

    if (json.has_value())
    {
      this->ParseSuggestions(json.value());
    }
    else
    { //no standard is defined. Suggestions should be empty
      m_Suggestions.clear();
    }
  }

  bool LabelSuggestionHelper::ParseSuggestions(const std::string& filePath, bool replaceExisting)
  {
    std::ifstream input(filePath);
    if (!input.is_open())
    {
      return false;
    }

    nlohmann::json fileContent;
    try
    {
      input >> fileContent;
    }
    catch (const nlohmann::json::parse_error& e)
    {
      mitkThrow() << "Cannot reader data due to parsing error. Parse error: " << e.what() << '\n';
    }

    //check version
    int version = 0;

    if (MultiLabelIOHelper::GetValueFromJson<int>(fileContent, "version", version))
    {
      if (version > MULTILABEL_SEGMENTATION_VERSION_VALUE)
      {
        mitkThrow() << "Suggestion file to parse has unsupported version. Software is to old to ensure correct reading. Please use a compatible version of MITK or store data in another format. Version of data: " << version << "; Supported versions up to: " << MULTILABEL_SEGMENTATION_VERSION_VALUE;
      }
    }
    else
    {
      MITK_INFO << "Data has unknown version. Assuming that it can be read. Result might be invalid.";
    }

    return this->ParseSuggestions(fileContent, replaceExisting);
  }

  bool LabelSuggestionHelper::ParseSuggestions(const nlohmann::json& jsonSuggestions, bool replaceExisting)
  {
    ConstLabelVectorType newSuggestions;

    auto labelGroups = MultiLabelIOHelper::DeserializeMultiLabelGroupsFromJSON(jsonSuggestions["groups"]);

    for (const auto& groupInfo : labelGroups)
    {
      MergeLabels(newSuggestions, MultiLabelSegmentation::ConvertLabelVectorConst(groupInfo.labels));
    }

    if (newSuggestions.empty())
    {
      MITK_WARN << "Failed to parse suggestions from: " << jsonSuggestions;
      return false;
    }

    if (replaceExisting)
    {
      m_Suggestions = newSuggestions;
    }
    else
    {
      MergeLabels(m_Suggestions, newSuggestions);
    }
    this->Modified();

    return true;
  }

  std::map<std::string, nlohmann::json> LabelSuggestionHelper::GetAllAvailableBuiltInSuggestions()
  {
    std::map<std::string, nlohmann::json> result;

    try
    {
      auto modules = us::GetModuleContext()->GetModules();

      for (auto& aModule : modules)
      {
        auto resources = aModule->FindResources("/LabelSuggestions","*.json",false);
        for (auto& resource : resources)
        {
          if (!resource) continue;

          std::string id = aModule->GetName() + ":" + resource.GetCompleteBaseName();

          us::ModuleResourceStream jsonStream(resource);

          nlohmann::json fileContent;
          try
          {
            jsonStream >> fileContent;
          }
          catch (const nlohmann::json::parse_error& e)
          {
            mitkThrow() << "Cannot read built-in config due to parsing error. Problematic resource: "<< id <<"; Parse error: " << e.what() << '\n';
          }

          result.emplace(id, fileContent);
        }
      }
    }
    catch (const std::exception& e)
    {
      MITK_WARN << "Failed to get all built-in suggestions: " << e.what();
      return {};
    }

    return result;
  }

  std::optional<nlohmann::json> LabelSuggestionHelper::GetStandardSuggesitions()
  {
    auto suggestionPref = mitk::LabelSuggestionHelper::GetSuggestionPreferences();

    std::optional<nlohmann::json> result;

    auto pos = suggestionPref.standardLabelSuggestionResource.find(':');
    std::string moduleName;
    std::string configName;

    if (pos == std::string::npos)
    {
      return result;
    }

    moduleName = suggestionPref.standardLabelSuggestionResource.substr(0, pos);
    configName = suggestionPref.standardLabelSuggestionResource.substr(pos + 1);

    try
    {
      auto aModule = us::GetModuleContext()->GetModule(moduleName);
      std::string filepath = "/LabelSuggestions/" + configName + ".json";

      auto resource = aModule->GetResource(filepath);

      if (!resource)
      {
        MITK_WARN << "Built-in suggestions indicated by preference could not be found. Preference: " << suggestionPref.standardLabelSuggestionResource << "; resource path: " << filepath;
      }

      us::ModuleResourceStream jsonStream(resource);

      nlohmann::json fileContent;
      try
      {
        jsonStream >> fileContent;
      }
      catch (const nlohmann::json::parse_error& e)
      {
        mitkThrow() << "Cannot read built-in config due to parsing error. Problematic resource: " << filepath << "; Parse error: " << e.what() << '\n';
      }

      result = fileContent;
    }
    catch (const std::exception& e)
    {
      MITK_WARN << "Failed to get built-in standard label suggestions: " << e.what();
    }
    return result;
  }

  LabelSuggestionHelper::ConstLabelVectorType LabelSuggestionHelper::GetValidSuggestionsForNewLabels(
    const MultiLabelSegmentation *segmentation) const
  {
    return FilterSuggestions(m_Suggestions, segmentation);
  }

  LabelSuggestionHelper::ConstLabelVectorType LabelSuggestionHelper::GetValidSuggestionsForRenamingLabels(
    const MultiLabelSegmentation *segmentation, const std::string_view labelName) const
  {
    if (segmentation == nullptr)
      mitkThrow() << "Invalid use of GetValidRenameSuggestions. Passed segmentation pointer is null.";
    if (labelName.empty())
      mitkThrow() << "Invalid use of GetValidRenameSuggestions. Passed label name is empty.";
    return FilterSuggestions(m_Suggestions, segmentation, labelName);
  }

  bool LabelSuggestionHelper::IsNewInstanceAllowed(const MultiLabelSegmentation *segmentation,
                                                   const std::string_view labelName) const
  {
    if (segmentation == nullptr)
      mitkThrow() << "Invalid use of IsNewInstanceAllowed. Passed segmentation pointer is null.";
    if (labelName.empty())
      mitkThrow() << "Invalid use of IsNewInstanceAllowed. Passed label name is empty.";

    auto maxInstance = GetMaxInstanceOccurrence(labelName);
    if (!maxInstance.has_value())
      return true; // Unlimited

    auto currentCount = segmentation->GetLabelValuesByName(labelName).size();
    return currentCount < maxInstance.value();
  }

  LabelSuggestionHelper::ConstLabelVectorType LabelSuggestionHelper::GetAllSuggestions() const
  {
    return m_Suggestions;
  }

  void LabelSuggestionHelper::ClearSuggestions()
  {
    m_Suggestions.clear();
    this->Modified();
  }


  void LabelSuggestionHelper::AddSuggestion(Label::Pointer label)
  {
    if (label.IsNull())
      return;

    m_Suggestions.push_back(label);
    this->Modified();
  }

  std::optional<unsigned int> LabelSuggestionHelper::GetMaxInstanceOccurrence(const std::string_view labelName) const
  {
    auto finding = std::find_if(this->m_Suggestions.begin(),
      this->m_Suggestions.end(),
      [labelName](const Label* l) { return labelName == l->GetName(); });

    if (finding == this->m_Suggestions.end())
    {
      return 0; // if not in the suggestions, not allowed at all.
    }
    return GetMaxInstanceOccurrenceOfLabel(finding->GetPointer(), GetSuggestionPreferences().suggestionOnce);
  }

  LabelSuggestionHelper::ConstLabelVectorType LabelSuggestionHelper::FilterSuggestions(
    const ConstLabelVectorType &suggestions,
    const MultiLabelSegmentation *segmentation,
    const std::optional<std::string_view> labelName)
  {
    if (segmentation == nullptr)
      return suggestions;

    LabelSuggestionHelper::ConstLabelVectorType filtered;

    for (const auto &suggestion : suggestions)
    {
      auto maxInstance = GetMaxInstanceOccurrenceOfLabel(suggestion, GetSuggestionPreferences().suggestionOnce);

      auto currentCount = segmentation->GetLabelValuesByName(suggestion->GetName()).size();

      if ((labelName.has_value() && labelName.value() == suggestion->GetName()) || //label is excluded from filtering
        !maxInstance.has_value() || //no limit is defined
        maxInstance.value() > currentCount) //limit is not reached
        filtered.push_back(suggestion);
    }

    return filtered;
  }

  LabelSuggestionHelper::Preferences LabelSuggestionHelper::GetSuggestionPreferences()
  {
    auto prefService = mitk::CoreServices::GetPreferencesService();

    Preferences prefs;

    if (nullptr != prefService)
    {
      auto systemPref = prefService->GetSystemPreferences();
      if (nullptr != systemPref)
      {
        auto* nodePrefs = systemPref->Node("/org.mitk.views.segmentation");

        prefs.externalLabelSuggestionFile = nodePrefs->Get("external label suggestions", prefs.externalLabelSuggestionFile);
        prefs.standardLabelSuggestionResource = nodePrefs->Get("standard label suggestions", prefs.standardLabelSuggestionResource);
        prefs.replaceStandardSuggestions = nodePrefs->GetBool("replace standard suggestions", prefs.replaceStandardSuggestions);
        prefs.enforceSuggestions = nodePrefs->GetBool("enforce suggestions", prefs.enforceSuggestions);
        prefs.suggestionOnce = nodePrefs->GetBool("suggest once", prefs.suggestionOnce);
      }
    }

    return prefs;
  }


} // namespace mitk
