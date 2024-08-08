/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkQuestionWithOptions.h>
#include <mitkExceptionMacro.h>

#include <nlohmann/json.hpp>

using namespace mitk::Forms;
using namespace nlohmann;

QuestionWithOptions::~QuestionWithOptions() = default;

void QuestionWithOptions::AddOption(const std::string& option)
{
  m_Options.push_back(option);
}

std::vector<std::string> QuestionWithOptions::GetOptions() const
{
  return m_Options;
}

void QuestionWithOptions::EnableOtherOption()
{
  m_EnableOtherOption = true;
}

bool QuestionWithOptions::HasOtherOption() const
{
  return m_EnableOtherOption;
}

void QuestionWithOptions::AddResponse(size_t i)
{
  if (std::find(m_Responses.begin(), m_Responses.end(), i) == m_Responses.end())
    m_Responses.insert(std::lower_bound(m_Responses.begin(), m_Responses.end(), i), i);
}

void QuestionWithOptions::RemoveResponse(size_t i)
{
  m_Responses.erase(std::remove(m_Responses.begin(), m_Responses.end(), i));
}

void QuestionWithOptions::SetResponse(size_t i)
{
  m_OtherResponse.reset();
  m_Responses = { i };
}

void QuestionWithOptions::AddOtherResponse(const std::string& response)
{
  if (!m_EnableOtherOption)
    mitkThrow() << "Adding \"Other\" response is disallowed!";

  m_OtherResponse = response;
}

void QuestionWithOptions::RemoveOtherResponse()
{
  m_OtherResponse.reset();
}

void QuestionWithOptions::SetOtherResponse(const std::string& response)
{
  if (!m_EnableOtherOption)
    mitkThrow() << "Setting \"Other\" response is disallowed!";

  m_Responses.clear();
  m_OtherResponse = response;
}

std::vector<std::string> QuestionWithOptions::GetResponsesAsStrings() const
{
  std::vector<std::string> responses;

  for (const auto i : m_Responses)
    responses.push_back(m_Options[i]);

  if (m_OtherResponse.has_value())
    responses.push_back(m_OtherResponse.value());

  return responses;
}

void QuestionWithOptions::ClearResponses()
{
  m_Responses.clear();
  m_OtherResponse.reset();
}

bool QuestionWithOptions::IsComplete() const
{
  return !m_Responses.empty() || m_OtherResponse.has_value();
}

void mitk::Forms::from_json(const ordered_json& j, QuestionWithOptions& q)
{
  from_json(j, static_cast<Question&>(q));

  if (j.contains("Options"))
  {
    for (const auto& option : j["Options"])
      q.AddOption(option);
  }
}

void mitk::Forms::to_json(ordered_json& j, const QuestionWithOptions& q)
{
  to_json(j, static_cast<const Question&>(q));

  j["Options"] = q.GetOptions();
}
