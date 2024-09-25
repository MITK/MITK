/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkScreenshotQuestion.h>
#include <nlohmann/json.hpp>

using namespace mitk::Forms;
using namespace nlohmann;

ScreenshotQuestion::~ScreenshotQuestion() = default;

std::string ScreenshotQuestion::GetType() const
{
  return "Screenshot";
}

Question* ScreenshotQuestion::CreateAnother() const
{
  return new ScreenshotQuestion;
}

void ScreenshotQuestion::FromJSON(const nlohmann::ordered_json& j)
{
  from_json(j, *this);
}

void ScreenshotQuestion::ToJSON(nlohmann::ordered_json& j) const
{
  to_json(j, *this);
}

std::vector<std::string> ScreenshotQuestion::GetResponsesAsStrings() const
{
  std::vector<std::string> responses;

  for (const auto& path : m_Responses)
    responses.push_back(path.string());

  return responses;
}

void ScreenshotQuestion::ClearResponses()
{
  m_Responses.clear();
}

bool ScreenshotQuestion::IsComplete() const
{
  return !m_Responses.empty();
}

bool ScreenshotQuestion::HasFileResponses() const
{
  return true;
}

std::vector<fs::path> ScreenshotQuestion::SubmitFileResponses(const fs::path& basePath) const
{
  fs::path destinationPath = "screenshots";
  fs::create_directories(basePath/destinationPath);

  std::vector<fs::path> result;

  for (const auto& sourcePath : m_Responses)
  {
    fs::copy_file(sourcePath, basePath/destinationPath/sourcePath.filename(), fs::copy_options::update_existing);
    result.push_back(destinationPath/sourcePath.filename());
  }

  return result;
}

void ScreenshotQuestion::AddScreenshot(const fs::path& path)
{
  if (fs::exists(path))
    m_Responses.insert(path);
}

void ScreenshotQuestion::RemoveScreenshot(const fs::path& path)
{
  try
  {
    if (m_Responses.erase(path) > 0 && fs::exists(path))
      fs::remove(path);
  }
  catch (const fs::filesystem_error&)
  {
  }
}

void mitk::Forms::from_json(const ordered_json& j, ScreenshotQuestion& q)
{
  from_json(j, static_cast<Question&>(q));
}

void mitk::Forms::to_json(ordered_json& j, const ScreenshotQuestion& q)
{
  to_json(j, static_cast<const Question&>(q));
}
