/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkShortAnswerQuestion.h>

using namespace mitk::Forms;
using namespace nlohmann;

ShortAnswerQuestion::~ShortAnswerQuestion() = default;

std::string ShortAnswerQuestion::GetType() const
{
  return "Short answer";
}

Question* ShortAnswerQuestion::CreateAnother() const
{
  return new ShortAnswerQuestion;
}

void ShortAnswerQuestion::FromJSON(const nlohmann::ordered_json& j)
{
  from_json(j, *this);
}

void ShortAnswerQuestion::ToJSON(nlohmann::ordered_json& j) const
{
  to_json(j, *this);
}

void mitk::Forms::from_json(const ordered_json& j, ShortAnswerQuestion& q)
{
  from_json(j, static_cast<Question&>(q));
}

void mitk::Forms::to_json(ordered_json& j, const ShortAnswerQuestion& q)
{
  to_json(j, static_cast<const Question&>(q));
}
