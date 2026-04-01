/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkQuestionWithOtherOption_h
#define mitkQuestionWithOtherOption_h

#include <mitkQuestionWithOptions.h>
#include <optional>

namespace mitk::Forms
{
  /** \brief Base class for questions with options that may also have a free text option.
   *
   * This base class is an extension of QuestionWithOptions representing questions that
   * may also have a free text option, typically labeled "Other" in forms/surveys.
   *
   * Just like with QuestionWithOptions, depending on the exclusivity of the answer
   * options, use using-declarations in derived classes to change the visibility of
   * the respective protected member functions to public, i.e., SetOtherResponse()
   * vs. AddOtherResponse() and RemoveOtherResponse().
   */
  class MITKFORMS_EXPORT QuestionWithOtherOption : public QuestionWithOptions
  {
  public:
    /** \brief Constructor.
     *
     * Initializes the question without an "Other" option.
     */
    QuestionWithOtherOption();

    /** \brief Virtual destructor.
     */
    ~QuestionWithOtherOption() override;

    /** \brief Return all selected responses as strings, including the "Other" response if set.
     *
     * \return A vector of response strings. The "Other" response, if present, is appended after
     *         the regular option responses.
     */
    std::vector<std::string> GetResponsesAsStrings() const override;

    /** \brief Clear all selected responses, including the "Other" response.
     */
    void ClearResponses() override;

    /** \brief Check whether at least one response (regular or "Other") has been given.
     *
     * \return \c true if at least one option is selected or the "Other" response is set.
     */
    bool IsComplete() const override;

    /** \brief Query whether this question actually has an "Other" option.
     *
     * \return \c true if the "Other" option has been enabled, \c false otherwise.
     */
    bool HasOtherOption() const;

    /** \brief Switch on the "Other" option.
     *
     * By default, a question does not have an "Other" option.
     */
    void EnableOtherOption();

  protected:
    /** \brief Set a regular option as the single response and clear any "Other" response.
     *
     * \param[in] i The zero-based index of the option to set.
     */
    void SetResponse(size_t i) override;

    /** \brief Add the free text given as "Other" option to the list of responses.
     *
     * \param[in] response The free text response.
     *
     * \pre HasOtherOption() must return \c true.
     * \throw mitk::Exception If the "Other" option is not enabled.
     *
     * \note A question can only have a single "Other" response. Consecutive calls
     * to this method will override the previously set "Other" response.
     */
    void AddOtherResponse(const std::string& response);

    /** \brief Remove the "Other" response from the list of already given responses.
     */
    void RemoveOtherResponse();

    /** \brief Set the "Other" response as single exclusive response to this question.
     *
     * This clears any previously selected regular option responses and sets only the
     * "Other" free text as the response.
     *
     * \param[in] response The free text response.
     *
     * \pre HasOtherOption() must return \c true.
     * \throw mitk::Exception If the "Other" option is not enabled.
     */
    void SetOtherResponse(const std::string& response);

  private:
    bool m_HasOtherOption;
    std::optional<std::string> m_OtherResponse;
  };

  /** \brief Deserialize a QuestionWithOtherOption from JSON.
   *
   * Deserializes the QuestionWithOptions fields and enables the "Other" option
   * if the "Other" key is present and \c true.
   *
   * \param[in] j The JSON object to deserialize from.
   * \param[in,out] q The QuestionWithOtherOption object to populate.
   *
   * \sa to_json(nlohmann::ordered_json&, const QuestionWithOtherOption&)
   */
  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, QuestionWithOtherOption& q);

  /** \brief Serialize a QuestionWithOtherOption to JSON.
   *
   * Serializes the QuestionWithOptions fields and adds an "Other" key if enabled.
   *
   * \param[out] j The JSON object to serialize into.
   * \param[in] q The QuestionWithOtherOption object to serialize.
   *
   * \sa from_json(const nlohmann::ordered_json&, QuestionWithOtherOption&)
   */
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const QuestionWithOtherOption& q);
}

#endif
