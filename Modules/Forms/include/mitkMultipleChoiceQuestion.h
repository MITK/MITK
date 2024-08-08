/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMultipleChoiceQuestion_h
#define mitkMultipleChoiceQuestion_h

#include <mitkQuestionWithOptions.h>

namespace mitk::Forms
{
  class MITKFORMS_EXPORT MultipleChoiceQuestion : public QuestionWithOptions
  {
  public:
    ~MultipleChoiceQuestion() override;

    std::string GetType() const override;
    Question* CreateAnother() const override;

    void FromJSON(const nlohmann::ordered_json& j) override;
    void ToJSON(nlohmann::ordered_json& j) const override;

    using QuestionWithOptions::SetResponse;
    using QuestionWithOptions::EnableOtherOption;
    using QuestionWithOptions::HasOtherOption;
    using QuestionWithOptions::SetOtherResponse;
  };

  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, MultipleChoiceQuestion& q);
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const MultipleChoiceQuestion& q);
}

#endif
