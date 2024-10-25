/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkQuestionWithOtherOption.h>
#include <mitkExceptionMacro.h>

#include <nlohmann/json.hpp>

using namespace mitk::Forms;
using namespace nlohmann;

QuestionWithOtherOption::QuestionWithOtherOption()
  : m_HasOtherOption(false)
{
}

QuestionWithOtherOption::~QuestionWithOtherOption() = default;

void QuestionWithOtherOption::EnableOtherOption()
{
  m_HasOtherOption = true;
}

bool QuestionWithOtherOption::HasOtherOption() const
{
  return m_HasOtherOption;
}

void QuestionWithOtherOption::SetResponse(size_t i)
{
  QuestionWithOptions::SetResponse(i);
  m_OtherResponse.reset();
}

void QuestionWithOtherOption::AddOtherResponse(const std::string& response)
{
  if (!m_HasOtherOption)
    mitkThrow() << "Adding \"Other\" response is disallowed!";

  m_OtherResponse = response;
}

void QuestionWithOtherOption::RemoveOtherResponse()
{
  m_OtherResponse.reset();
}

void QuestionWithOtherOption::SetOtherResponse(const std::string& response)
{
  if (!m_HasOtherOption)
    mitkThrow() << "Setting \"Other\" response is disallowed!";

  QuestionWithOptions::ClearResponses();
  m_OtherResponse = response;
}

std::vector<std::string> QuestionWithOtherOption::GetResponsesAsStrings() const
{
  auto responses = QuestionWithOptions::GetResponsesAsStrings();

  if (m_OtherResponse.has_value())
    responses.push_back(m_OtherResponse.value());

  return responses;
}

void QuestionWithOtherOption::ClearResponses()
{
  QuestionWithOptions::ClearResponses();
  m_OtherResponse.reset();
}

bool QuestionWithOtherOption::IsComplete() const
{
  return QuestionWithOptions::IsComplete() || m_OtherResponse.has_value();
}

void mitk::Forms::from_json(const ordered_json& j, QuestionWithOtherOption& q)
{
  from_json(j, static_cast<QuestionWithOptions&>(q));

  if (j.contains("Other") && j["Other"] == true)
    q.EnableOtherOption();
}

void mitk::Forms::to_json(ordered_json& j, const QuestionWithOtherOption& q)
{
  to_json(j, static_cast<const QuestionWithOptions&>(q));

  if (q.HasOtherOption())
    j["Other"] = true;
}
