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
#include <optional>

namespace mitk::Forms
{
  class MITKFORMS_EXPORT QuestionWithOptions : public Question
  {
  public:
    ~QuestionWithOptions() override;

    std::vector<std::string> GetResponsesAsStrings() const override;
    void ClearResponses() override;

    bool IsComplete() const override;

    void AddOption(const std::string& option);
    std::vector<std::string> GetOptions() const;

  protected:
    void AddResponse(size_t i);
    void RemoveResponse(size_t i);

    virtual void SetResponse(size_t i);

  private:
    std::vector<std::string> m_Options;
    std::vector<size_t> m_Responses;
  };

  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, QuestionWithOptions& q);
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const QuestionWithOptions& q);
}

#endif
