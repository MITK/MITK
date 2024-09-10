/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkParagraphQuestion.h>

using namespace mitk::Forms;
using namespace nlohmann;

ParagraphQuestion::~ParagraphQuestion() = default;

std::string ParagraphQuestion::GetType() const
{
  return "Paragraph";
}

Question* ParagraphQuestion::CreateAnother() const
{
  return new ParagraphQuestion;
}

void ParagraphQuestion::FromJSON(const nlohmann::ordered_json& j)
{
  from_json(j, *this);
}

void ParagraphQuestion::ToJSON(nlohmann::ordered_json& j) const
{
  to_json(j, *this);
}

void mitk::Forms::from_json(const ordered_json& j, ParagraphQuestion& q)
{
  from_json(j, static_cast<Question&>(q));
}

void mitk::Forms::to_json(ordered_json& j, const ParagraphQuestion& q)
{
  to_json(j, static_cast<const Question&>(q));
}
