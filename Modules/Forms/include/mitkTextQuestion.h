/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTextQuestion_h
#define mitkTextQuestion_h

#include <mitkQuestion.h>

namespace mitk::Forms
{
  /** \brief Abstract base class for questions with a single free-text response.
   *
   * This intermediate class provides shared response storage and accessors for
   * question types whose answer is a plain text string.
   *
   * \sa ParagraphQuestion, ShortAnswerQuestion
   */
  class MITKFORMS_EXPORT TextQuestion : public Question
  {
  public:
    /** \brief Virtual destructor.
     */
    ~TextQuestion() override;

    /** \brief Return the text response as a single-element vector.
     *
     * \return A vector containing exactly one string (which may be empty).
     */
    std::vector<std::string> GetResponsesAsStrings() const override;

    /** \brief Clear the text response.
     */
    void ClearResponses() override;

    /** \brief Check whether the response is non-empty.
     *
     * \return \c true if the response string is not empty, \c false otherwise.
     */
    bool IsComplete() const override;

    /** \brief Get the current text response.
     *
     * \return The response string.
     */
    std::string GetResponse() const;

    /** \brief Set the text response.
     *
     * \param[in] response The response string.
     */
    void SetResponse(const std::string& response);

  private:
    std::string m_Response;
  };
}

#endif
