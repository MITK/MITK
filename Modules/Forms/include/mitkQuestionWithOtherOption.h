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
    QuestionWithOtherOption();
    ~QuestionWithOtherOption() override;

    std::vector<std::string> GetResponsesAsStrings() const override;
    void ClearResponses() override;

    bool IsComplete() const override;

    /** \brief Query whether this question actually has an "Other" option.
     */
    bool HasOtherOption() const;

    /** \brief Switch on the "Other" option.
     *
     * By default, a question does not have an "Other" option.
     */
    void EnableOtherOption();

  protected:
    void SetResponse(size_t i) override;

    /** \brief Add the free text given as "Other" option to the list of responses.
     *
     * \note A question can only have a single "Other" response. Consequtive calls
     * to this method will override the previously set "Other" response.
     */
    void AddOtherResponse(const std::string& response);

    /** \brief Remove the "Other" response from the list of already given responses.
     */
    void RemoveOtherResponse();

    /** \brief Set the "Other" response as single exclusive response to this question.
     */
    void SetOtherResponse(const std::string& response);

  private:
    bool m_HasOtherOption;
    std::optional<std::string> m_OtherResponse;
  };

  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, QuestionWithOtherOption& q);
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const QuestionWithOtherOption& q);
}

#endif
