/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLabelSuggestionHelper_h
#define mitkLabelSuggestionHelper_h

#include <MitkMultilabelExports.h>

#include <mitkLabel.h>
#include <mitkLabelSetImage.h>

#include <nlohmann/json.hpp>

#include <string>
#include <string_view>
#include <vector>
#include <optional>

namespace mitk
{
  /**
   * @brief Helper class for handling label suggestions with constraint checking.
   *
   * This class provides functionality to:
   * - Parse label suggestions from JSON files (both new MITK stack format and legacy format)
   * - Filter suggestions based on occurrence constraints
   * - Determine valid suggestions for adding new labels (instances)
   * - Determine valid suggestions for renaming existing labels
   * - Check if new label instances are allowed
   *
   * The helper supports special label properties:
   * - _max_instance_occurrence: Maximum number of instances of a specific label
   */

  class MITKMULTILABEL_EXPORT LabelSuggestionHelper : public itk::Object
  {
  public:
    mitkClassMacroItkParent(LabelSuggestionHelper, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    // Property names for occurrence constraints
    static constexpr const char* PROPERTY_MAX_INSTANCE_OCCURRENCE = "_max_instance_occurrence";

    using ConstLabelVectorType = MultiLabelSegmentation::ConstLabelVectorType;

    /**
     * @brief Parse label suggestions from a JSON file.
     * @param filePath Path to the JSON file
     * @param replaceExisting If true, replace all existing suggestions; if false, merge with existing
     * @return true if parsing was successful, false otherwise
     */
    bool ParseSuggestions(const std::string& filePath, bool replaceExisting = true);

    /**
     * @brief Parse label suggestions from a JSON object.
     * @param jsonSuggestions suggestions stored as json object
     * @param replaceExisting If true, replace all existing suggestions; if false, merge with existing
     * @return true if parsing was successful, false otherwise
     */
    bool ParseSuggestions(const nlohmann::json& jsonSuggestions, bool replaceExisting = true);

    /**
     * @brief Resets suggestions and load standard suggestions from AnatomicalStructureColorPresets.
     *
     */
    void LoadStandardSuggestions();

    /**
     * @brief Get valid suggestions for adding new labels to a segmentation.
     *
     * Filters suggestions based on:
     * - Occurrence constraints (_max_class_occurrence, _max_instance_occurrence)
     * - Suggest-once policy (if enabled)
     * - Existing labels in the segmentation
     *
     * @param segmentation The segmentation to check against (can be nullptr)
     * @param suggestOnce If true, filter out suggestions already used in the segmentation
     * @return Vector of valid label suggestions
     */
    ConstLabelVectorType GetValidSuggestionsForNewLabels(const MultiLabelSegmentation* segmentation = nullptr,
                                                        bool suggestOnce = true) const;

    /**
     * @brief Get valid suggestions for renaming a specific label.
     *
     * Returns suggestions that would be valid if the given label were renamed.
     * Takes into account that renaming removes the old label name from the count.
     *
     * @param segmentation The segmentation containing the label
     * @param labelName The label name that should be checked for renaming
     * @param suggestOnce If true, filter out suggestions already used (except the label's current name)
     * @return Vector of valid label suggestions for renaming
     */
    ConstLabelVectorType GetValidSuggestionsForRenamingLabels(const MultiLabelSegmentation* segmentation,
                                                   const std::string_view labelName,
                                                   bool suggestOnce = true) const;

    /**
     * @brief Check if a new instance of a label is allowed.
     *
     * Checks the _max_instance_occurrence constraint if present.
     *
     * @param segmentation The segmentation to check against
     * @param labelName The label name that should be checked for if another instance is allowed.
     * @return true if a new instance is allowed, false otherwise
     */
    bool IsNewInstanceAllowed(const MultiLabelSegmentation* segmentation, const std::string_view labelName) const;

    /**
     * @brief Get all currently loaded suggestions (unfiltered).
     *
     * @return Vector of all suggestions
     */
    ConstLabelVectorType GetAllSuggestions() const;

    /**
     * @brief Clear all loaded suggestions.
     */
    void ClearSuggestions();

    /**
     * @brief Add a single suggestion manually.
     *
     * @param label The label to add as a suggestion
     */
    void AddSuggestion(Label::Pointer label);

    struct Preferences
    {
      std::string labelSuggestionFile = "";
      bool replaceStandardSuggestions = true; ///< indicates if standard suggestions should be kept
      bool enforceSuggestions = false; ///< indicates if only suggestions are allowed or if users can define own label names
      /// the global preference settings of suggestions are only allowed once if the max multiplicity
      /// is not explicitly specified for a label suggestions.
      bool suggestionOnce = true;
    };

    // Get all relevant preferences and consider command-line arguments overrides.
    static Preferences GetSuggestionPreferences();

  protected:
    LabelSuggestionHelper();
    ~LabelSuggestionHelper();
    LabelSuggestionHelper(const LabelSuggestionHelper&) = delete;
    LabelSuggestionHelper& operator= (const LabelSuggestionHelper&) = delete;

  private:
    /**
     * @brief Get maximum instance occurrence constraint from label properties.
     *
     * @param labelName Name of the label in the suggestions to check
     * @return Maximum occurrences, optional has no value if unlimited
     */
    std::optional<unsigned int> GetMaxInstanceOccurrence(const std::string_view labelname) const;

    /**
     * @brief Filter suggestions based on constraints and existing labels.
     *
     * @param suggestions All suggestions to filter
     * @param segmentation Current segmentation (can be nullptr)
     * @param suggestOnce If true, filter out already used suggestions
     * @param excludeLabel Optional label to exclude from counting (used for rename)
     * @return Filtered suggestions
     */
    static ConstLabelVectorType FilterSuggestions(const ConstLabelVectorType& suggestions,
                                                   const MultiLabelSegmentation* segmentation,
                                                   const std::optional<std::string_view> labelName = std::optional<std::string_view>());

    ConstLabelVectorType m_Suggestions;
  };

} // namespace mitk

#endif // mitkLabelSuggestionHelper_h
