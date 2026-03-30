/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCheckboxesQuestion_h
#define mitkCheckboxesQuestion_h

#include <mitkQuestionWithOtherOption.h>

namespace mitk::Forms
{
  /** \brief A Question whose possible responses are represented by checkboxes.
   *
   * The question can have multiple responses at once, including an optional
   * free text response.
   *
   * \sa MultipleChoiceQuestion, DropdownQuestion
   */
  class MITKFORMS_EXPORT CheckboxesQuestion : public QuestionWithOtherOption
  {
  public:
    /** \brief Virtual destructor.
     */
    ~CheckboxesQuestion() override;

    /** \brief Return the type string "Checkboxes".
     *
     * \return The string "Checkboxes".
     */
    std::string GetType() const override;

    /** \brief Create a new default-constructed CheckboxesQuestion instance.
     *
     * \return A pointer to a new CheckboxesQuestion. The caller takes ownership.
     */
    Question* CreateAnother() const override;

    /** \brief Deserialize this CheckboxesQuestion from JSON.
     *
     * \param[in] j The JSON object to deserialize from.
     */
    void FromJSON(const nlohmann::ordered_json& j) override;

    /** \brief Serialize this CheckboxesQuestion to JSON.
     *
     * \param[out] j The JSON object to serialize into.
     */
    void ToJSON(nlohmann::ordered_json& j) const override;

    /** \brief Add one of the regular options to the set of selected responses.
     *
     * Exposed from QuestionWithOptions. Multiple options may be selected simultaneously.
     */
    using QuestionWithOptions::AddResponse;

    /** \brief Remove one of the regular options from the set of selected responses.
     *
     * Exposed from QuestionWithOptions.
     */
    using QuestionWithOptions::RemoveResponse;

    /** \brief Add or update the "Other" free text response alongside regular selections.
     *
     * Exposed from QuestionWithOtherOption.
     */
    using QuestionWithOtherOption::AddOtherResponse;

    /** \brief Remove the "Other" free text response.
     *
     * Exposed from QuestionWithOtherOption.
     */
    using QuestionWithOtherOption::RemoveOtherResponse;
  };

  /** \brief Deserialize a CheckboxesQuestion from JSON.
   *
   * \param[in] j The JSON object to deserialize from.
   * \param[in,out] q The CheckboxesQuestion to populate.
   *
   * \sa to_json(nlohmann::ordered_json&, const CheckboxesQuestion&)
   */
  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, CheckboxesQuestion& q);

  /** \brief Serialize a CheckboxesQuestion to JSON.
   *
   * \param[out] j The JSON object to serialize into.
   * \param[in] q The CheckboxesQuestion to serialize.
   *
   * \sa from_json(const nlohmann::ordered_json&, CheckboxesQuestion&)
   */
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const CheckboxesQuestion& q);
}

#endif
