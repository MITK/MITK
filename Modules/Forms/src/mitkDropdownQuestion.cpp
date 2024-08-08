/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDropdownQuestion.h>

using namespace mitk::Forms;
using namespace nlohmann;

DropdownQuestion::~DropdownQuestion() = default;

std::string DropdownQuestion::GetType() const
{
  return "Drop-down";
}

Question* DropdownQuestion::CreateAnother() const
{
  return new DropdownQuestion;
}

void DropdownQuestion::FromJSON(const nlohmann::ordered_json& j)
{
  from_json(j, *this);
}

void DropdownQuestion::ToJSON(nlohmann::ordered_json& j) const
{
  to_json(j, *this);
}

void mitk::Forms::from_json(const ordered_json& j, DropdownQuestion& q)
{
  from_json(j, static_cast<QuestionWithOptions&>(q));
}

void mitk::Forms::to_json(ordered_json& j, const DropdownQuestion& q)
{
  to_json(j, static_cast<const QuestionWithOptions&>(q));
}
