/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCheckboxesQuestion.h>

using namespace mitk::Forms;
using namespace nlohmann;

CheckboxesQuestion::~CheckboxesQuestion() = default;

std::string CheckboxesQuestion::GetType() const
{
  return "Checkboxes";
}

Question* CheckboxesQuestion::CreateAnother() const
{
  return new CheckboxesQuestion;
}

void CheckboxesQuestion::FromJSON(const nlohmann::ordered_json& j)
{
  from_json(j, *this);
}

void CheckboxesQuestion::ToJSON(nlohmann::ordered_json& j) const
{
  to_json(j, *this);
}

void mitk::Forms::from_json(const ordered_json& j, CheckboxesQuestion& q)
{
  from_json(j, static_cast<QuestionWithOtherOption&>(q));
}

void mitk::Forms::to_json(ordered_json& j, const CheckboxesQuestion& q)
{
  to_json(j, static_cast<const QuestionWithOtherOption&>(q));
}
