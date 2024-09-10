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
    ~MultipleChoiceQuestion() override;

    std::string GetType() const override;
    Question* CreateAnother() const override;

    void FromJSON(const nlohmann::ordered_json& j) override;
    void ToJSON(nlohmann::ordered_json& j) const override;

    using QuestionWithOptions::SetResponse;
    using QuestionWithOtherOption::SetOtherResponse;
  };

  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, MultipleChoiceQuestion& q);
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const MultipleChoiceQuestion& q);
}

#endif
