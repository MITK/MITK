/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDropdownQuestion_h
#define mitkDropdownQuestion_h

#include <mitkQuestionWithOptions.h>

namespace mitk::Forms
{
  /** \brief A Question whose possible responses are represented by a combo box.
   *
   * The question can have a single response.
   *
   * \sa MultipleChoiceQuestion, CheckboxesQuestion
   */
  class MITKFORMS_EXPORT DropdownQuestion : public QuestionWithOptions
  {
  public:
    /** \brief Virtual destructor.
     */
    ~DropdownQuestion() override;

    /** \brief Return the type string "Drop-down".
     *
     * \return The string "Drop-down".
     */
    std::string GetType() const override;

    /** \brief Create a new default-constructed DropdownQuestion instance.
     *
     * \return A pointer to a new DropdownQuestion. The caller takes ownership.
     */
    Question* CreateAnother() const override;

    /** \brief Deserialize this DropdownQuestion from JSON.
     *
     * \param[in] j The JSON object to deserialize from.
     */
    void FromJSON(const nlohmann::ordered_json& j) override;

    /** \brief Serialize this DropdownQuestion to JSON.
     *
     * \param[out] j The JSON object to serialize into.
     */
    void ToJSON(nlohmann::ordered_json& j) const override;

    /** \brief Set one of the possible answer options as the single response.
     *
     * Exposed from QuestionWithOptions. Only a single option may be selected.
     */
    using QuestionWithOptions::SetResponse;
  };

  /** \brief Deserialize a DropdownQuestion from JSON.
   *
   * \param[in] j The JSON object to deserialize from.
   * \param[in,out] q The DropdownQuestion to populate.
   *
   * \sa to_json(nlohmann::ordered_json&, const DropdownQuestion&)
   */
  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, DropdownQuestion& q);

  /** \brief Serialize a DropdownQuestion to JSON.
   *
   * \param[out] j The JSON object to serialize into.
   * \param[in] q The DropdownQuestion to serialize.
   *
   * \sa from_json(const nlohmann::ordered_json&, DropdownQuestion&)
   */
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const DropdownQuestion& q);
}

#endif
