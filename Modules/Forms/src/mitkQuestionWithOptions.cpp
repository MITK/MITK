/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkQuestionWithOptions.h>
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
  m_Responses = { i };
}

std::vector<std::string> QuestionWithOptions::GetResponsesAsStrings() const
{
  std::vector<std::string> responses;

  for (const auto i : m_Responses)
    responses.push_back(m_Options[i]);

  return responses;
}

void QuestionWithOptions::ClearResponses()
{
  m_Responses.clear();
}

bool QuestionWithOptions::IsComplete() const
{
  return !m_Responses.empty();
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
