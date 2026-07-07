/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkQuestionWithOptions_h
#define mitkQuestionWithOptions_h

#include <mitkQuestion.h>
#include <set>

namespace mitk::Forms
{
  /** \brief Base class for questions with options to choose from as response.
   *
   * This base class represents both, questions with mutually exclusive options as well as
   * questions with multiple answer options. Derived classes should use using-declarations
   * to change the visibility of the respective protected member functions to public, i.e.,
   * SetResponse() vs. AddResponse() and RemoveResponse().
   */
  class MITKFORMS_EXPORT QuestionWithOptions : public Question
  {
  public:
    /** \brief Virtual destructor.
     */
    ~QuestionWithOptions() override;

    /** \brief Return the selected options as strings.
     *
     * The returned strings correspond to the option texts at the stored response indices,
     * ordered in ascending index order.
     *
     * \return A vector of selected option strings.
     */
    std::vector<std::string> GetResponsesAsStrings() const override;

    /** \brief Clear all selected responses.
     */
    void ClearResponses() override;

    /** \brief Check whether at least one option has been selected.
     *
     * \return \c true if at least one response index is stored, \c false otherwise.
     */
    bool IsComplete() const override;

    /** \brief Add a non-exclusive option as possible answer to the question.
     *
     * The option is appended to the list of already existing options.
     *
     * \param[in] option The option text.
     * \return The zero-based index of the newly added option.
     */
    size_t AddOption(const std::string& option);

    /** \brief Get all available options.
     *
     * \return A vector of option strings in the order they were added.
     */
    std::vector<std::string> GetOptions() const;

  protected:
    /** \brief Add one of the possible answer options to the responses.
     *
     * Indexes to responses are inserted in ascending order.
     *
     * \param[in] i The zero-based index of the option to add to responses.
     *
     * \sa AddOption()
     */
    void AddResponse(size_t i);

    /** \brief Remove one of the already given responses.
     *
     * \param[in] i The zero-based index of the option to remove from responses.
     *
     * \sa AddOption(), AddResponse()
     */
    void RemoveResponse(size_t i);

    /** \brief Set one of the possible answer options as the single response.
     *
     * \param[in] i The zero-based index of the option to set as sole response.
     *
     * \note This will remove any responses added by AddResponse().
     */
    virtual void SetResponse(size_t i);

  private:
    std::vector<std::string> m_Options;
    std::set<size_t> m_Responses;
  };

  /** \brief Deserialize a QuestionWithOptions from JSON.
   *
   * Deserializes the base Question fields and the "Options" array.
   *
   * \param[in] j The JSON object to deserialize from.
   * \param[in,out] q The QuestionWithOptions object to populate.
   *
   * \sa to_json(nlohmann::ordered_json&, const QuestionWithOptions&)
   */
  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, QuestionWithOptions& q);

  /** \brief Serialize a QuestionWithOptions to JSON.
   *
   * Serializes the base Question fields and the "Options" array.
   *
   * \param[out] j The JSON object to serialize into.
   * \param[in] q The QuestionWithOptions object to serialize.
   *
   * \sa from_json(const nlohmann::ordered_json&, QuestionWithOptions&)
   */
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const QuestionWithOptions& q);
}

#endif
