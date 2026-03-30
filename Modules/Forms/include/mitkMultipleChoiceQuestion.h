/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultipleChoiceQuestion_h
#define mitkMultipleChoiceQuestion_h

#include <mitkQuestionWithOtherOption.h>

namespace mitk::Forms
{
  /** \brief A Question whose possible responses are represented by radio buttons.
   *
   * The question can have a single response, which can optionally be free text.
   *
   * \sa CheckboxesQuestion, DropdownQuestion
   */
  class MITKFORMS_EXPORT MultipleChoiceQuestion : public QuestionWithOtherOption
  {
  public:
    /** \brief Virtual destructor.
     */
    ~MultipleChoiceQuestion() override;

    /** \brief Return the type string "Multiple choice".
     *
     * \return The string "Multiple choice".
     */
    std::string GetType() const override;

    /** \brief Create a new default-constructed MultipleChoiceQuestion instance.
     *
     * \return A pointer to a new MultipleChoiceQuestion. The caller takes ownership.
     */
    Question* CreateAnother() const override;

    /** \brief Deserialize this MultipleChoiceQuestion from JSON.
     *
     * \param[in] j The JSON object to deserialize from.
     */
    void FromJSON(const nlohmann::ordered_json& j) override;

    /** \brief Serialize this MultipleChoiceQuestion to JSON.
     *
     * \param[out] j The JSON object to serialize into.
     */
    void ToJSON(nlohmann::ordered_json& j) const override;

    /** \brief Set one of the regular options as the single exclusive response.
     *
     * Exposed from QuestionWithOptions. Clears any "Other" response.
     */
    using QuestionWithOptions::SetResponse;

    /** \brief Set the "Other" free text as the single exclusive response.
     *
     * Exposed from QuestionWithOtherOption. Clears any regular option responses.
     */
    using QuestionWithOtherOption::SetOtherResponse;
  };

  /** \brief Deserialize a MultipleChoiceQuestion from JSON.
   *
   * \param[in] j The JSON object to deserialize from.
   * \param[in,out] q The MultipleChoiceQuestion to populate.
   *
   * \sa to_json(nlohmann::ordered_json&, const MultipleChoiceQuestion&)
   */
  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, MultipleChoiceQuestion& q);

  /** \brief Serialize a MultipleChoiceQuestion to JSON.
   *
   * \param[out] j The JSON object to serialize into.
   * \param[in] q The MultipleChoiceQuestion to serialize.
   *
   * \sa from_json(const nlohmann::ordered_json&, MultipleChoiceQuestion&)
   */
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const MultipleChoiceQuestion& q);
}

#endif
