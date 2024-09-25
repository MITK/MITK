/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkQuestion.h>
#include <nlohmann/json.hpp>

using namespace mitk::Forms;
using namespace nlohmann;

Question::Question()
  : m_IsRequired(false)
{
}

Question::~Question() = default;

std::string Question::GetQuestionText() const
{
  return m_QuestionText;
}

void Question::SetQuestionText(const std::string& question)
{
  m_QuestionText = question;
}

void Question::SetRequired(bool required)
{
  m_IsRequired = required;
}

bool Question::IsRequired() const
{
  return m_IsRequired;
}

std::string Question::GetRequiredText() const
{
  return "This is a required question";
}

bool Question::HasFileResponses() const
{
  return false;
}

std::vector<fs::path> Question::SubmitFileResponses(const fs::path& basePath) const
{
  return {};
}

void mitk::Forms::from_json(const ordered_json& j, Question& q)
{
  if (j.contains("Text"))
    q.SetQuestionText(j["Text"]);

  if (j.contains("Required"))
    q.SetRequired(j["Required"]);
}

void mitk::Forms::to_json(ordered_json& j, const Question& q)
{
  j["Text"] = q.GetQuestionText();

  if (q.IsRequired())
    j["Required"] = true;

  j["Type"] = q.GetType();
}
