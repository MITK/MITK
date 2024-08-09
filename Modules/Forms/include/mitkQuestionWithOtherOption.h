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
  class MITKFORMS_EXPORT QuestionWithOtherOption : public QuestionWithOptions
  {
  public:
    QuestionWithOtherOption();
    ~QuestionWithOtherOption() override;

    std::vector<std::string> GetResponsesAsStrings() const override;
    void ClearResponses() override;

    bool IsComplete() const override;

    bool HasOtherOption() const;
    void EnableOtherOption();

  protected:
    void SetResponse(size_t i) override;

    void AddOtherResponse(const std::string& response);
    void RemoveOtherResponse();

    void SetOtherResponse(const std::string& response);

  private:
    bool m_HasOtherOption;
    std::optional<std::string> m_OtherResponse;
  };

  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, QuestionWithOtherOption& q);
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const QuestionWithOtherOption& q);
}

#endif
