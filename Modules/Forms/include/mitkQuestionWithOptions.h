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
    ~QuestionWithOptions() override;

    std::vector<std::string> GetResponsesAsStrings() const override;
    void ClearResponses() override;

    bool IsComplete() const override;

    /** \brief Add a non-exclusive option as possible answer to the question.
     *
     * The option is appended to the list of already existing options.
     *
     * \return The index of the option
     */
    size_t AddOption(const std::string& option);

    std::vector<std::string> GetOptions() const;

  protected:
    /** \brief Add one of the possible answer options to the responses.
     *
     * Indexes to responses are inserted in ascending order.
     *
     * \sa AddOption()
     */
    void AddResponse(size_t i);

    /** \brief Remove one of the already given responses.
     *
     * \sa AddOption(), AddResponse()
     */
    void RemoveResponse(size_t i);

    /** \brief Set one of the possible answer options as the single response.
     *
     * \note This will remove any responses added by AddResponse().
     */
    virtual void SetResponse(size_t i);

  private:
    std::vector<std::string> m_Options;
    std::set<size_t> m_Responses;
  };

  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, QuestionWithOptions& q);
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const QuestionWithOptions& q);
}

#endif
