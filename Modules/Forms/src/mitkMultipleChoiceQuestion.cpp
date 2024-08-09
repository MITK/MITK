/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMultipleChoiceQuestion.h>

using namespace mitk::Forms;
using namespace nlohmann;

MultipleChoiceQuestion::~MultipleChoiceQuestion() = default;

std::string MultipleChoiceQuestion::GetType() const
{
  return "Multiple choice";
}

Question* MultipleChoiceQuestion::CreateAnother() const
{
  return new MultipleChoiceQuestion;
}

void MultipleChoiceQuestion::FromJSON(const nlohmann::ordered_json& j)
{
  from_json(j, *this);
}

void MultipleChoiceQuestion::ToJSON(nlohmann::ordered_json& j) const
{
  to_json(j, *this);
}

void mitk::Forms::from_json(const ordered_json& j, MultipleChoiceQuestion& q)
{
  from_json(j, static_cast<QuestionWithOtherOption&>(q));
}

void mitk::Forms::to_json(ordered_json& j, const MultipleChoiceQuestion& q)
{
  to_json(j, static_cast<const QuestionWithOtherOption&>(q));
}
