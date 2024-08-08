/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkLinearScaleQuestion.h>
#include <mitkExceptionMacro.h>

#include <nlohmann/json.hpp>

using namespace mitk::Forms;
using namespace nlohmann;

LinearScaleQuestion::LinearScaleQuestion()
  : m_Range({1, 5})
{
}

LinearScaleQuestion::~LinearScaleQuestion() = default;

std::string LinearScaleQuestion::GetType() const
{
  return "Linear scale";
}

Question* LinearScaleQuestion::CreateAnother() const
{
  return new LinearScaleQuestion;
}

void LinearScaleQuestion::FromJSON(const nlohmann::ordered_json& j)
{
  from_json(j, *this);
}

void LinearScaleQuestion::ToJSON(nlohmann::ordered_json& j) const
{
  to_json(j, *this);
}

std::vector<std::string> LinearScaleQuestion::GetResponsesAsStrings() const
{
  std::vector<std::string> responses;

  if (m_Response.has_value())
    responses.push_back(std::to_string(m_Response.value()));

  return responses;
}

void LinearScaleQuestion::ClearResponses()
{
  m_Response.reset();
}

bool LinearScaleQuestion::IsComplete() const
{
  return m_Response.has_value();
}

std::optional<int> LinearScaleQuestion::GetResponse() const
{
  return m_Response;
}

void LinearScaleQuestion::SetResponse(int response)
{
  m_Response = response;
}

std::pair<int, int> LinearScaleQuestion::GetRange() const
{
  return m_Range;
}

void LinearScaleQuestion::SetRange(const std::pair<int, int>& range)
{
  if (range.first < 0 || range.first > 1)
    mitkThrow() << "Invalid lower bound " << range.first << " in range: must be 0 or 1!";

  if (range.second < 2 || range.second > 10)
    mitkThrow() << "Invalid upper bound " << range.second << " in range: must lie between 2 and 10!";

  m_Range = range;
}

std::pair<std::string, std::string> LinearScaleQuestion::GetRangeLabels() const
{
  return m_Labels;
}

void LinearScaleQuestion::SetRangeLabels(const std::pair<std::string, std::string>& labels)
{
  m_Labels = labels;
}

void mitk::Forms::from_json(const ordered_json& j, LinearScaleQuestion& q)
{
  from_json(j, static_cast<Question&>(q));

  if (j.contains("Range"))
    q.SetRange(j["Range"]);

  if (j.contains("Labels"))
    q.SetRangeLabels(j["Labels"]);
}

void mitk::Forms::to_json(ordered_json& j, const LinearScaleQuestion& q)
{
  to_json(j, static_cast<const Question&>(q));

  j["Range"] = q.GetRange();
  j["Labels"] = q.GetRangeLabels();
}
