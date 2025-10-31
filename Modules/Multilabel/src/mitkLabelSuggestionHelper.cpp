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

  void LabelSuggestionHelper::LoadStandardSuggestions()
  {
    try
    {
      us::ModuleResource presetResource = us::GetModuleContext()->GetModule("MitkCore")->GetResource("mitkLabelSuggestions_classic.json");
      if (!presetResource) return;

      us::ModuleResourceStream presetStream(presetResource);

      nlohmann::json fileContent;
      try
      {
        presetStream >> fileContent;
      }
      catch (const nlohmann::json::parse_error& e)
      {
        mitkThrow() << "Cannot reader data due to parsing error. Parse error: " << e.what() << '\n';
      }

      this->ParseSuggestions(fileContent, true);
    }
    catch (const std::exception &e)
    {
      MITK_WARN << "Failed to load standard suggestions: " << e.what();
      return;
    }
  }

  LabelSuggestionHelper::ConstLabelVectorType LabelSuggestionHelper::GetValidSuggestionsForNewLabels(
    const MultiLabelSegmentation *segmentation, bool suggestOnce) const
  {
    return FilterSuggestions(m_Suggestions, segmentation);
  }

  LabelSuggestionHelper::ConstLabelVectorType LabelSuggestionHelper::GetValidSuggestionsForRenamingLabels(
    const MultiLabelSegmentation *segmentation, const std::string_view labelName, bool suggestOnce) const
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
      bool shouldInclude = true;

      auto maxInstance = GetMaxInstanceOccurrenceOfLabel(suggestion, GetSuggestionPreferences().suggestionOnce);

      auto currentCount = segmentation->GetLabelValuesByName(suggestion->GetName()).size();
      auto hasLimit = !maxInstance.has_value();

      if ((labelName.has_value() && labelName.value() == suggestion->GetName()) || //label is excluded from filtering
        !maxInstance.has_value() || //no limit is defined
        maxInstance.value() > currentCount) //limit is not reached
        filtered.push_back(suggestion);
    }

    return filtered;
  }

  LabelSuggestionHelper::Preferences LabelSuggestionHelper::GetSuggestionPreferences()
  {
    auto* nodePrefs = mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node("/org.mitk.views.segmentation");

    Preferences prefs;

    prefs.labelSuggestionFile = nodePrefs->Get("label suggestions", "");
    prefs.replaceStandardSuggestions = nodePrefs->GetBool("replace standard suggestions", true);
    prefs.enforceSuggestions = nodePrefs->GetBool("enforce suggestions", false);
    prefs.suggestionOnce = nodePrefs->GetBool("suggest once", true);

    return prefs;
  }


} // namespace mitk
