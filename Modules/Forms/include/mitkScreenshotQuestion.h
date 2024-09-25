/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkScreenshotQuestion_h
#define mitkScreenshotQuestion_h

#include <mitkQuestion.h>
#include <mitkFileSystem.h>

#include <set>

namespace mitk::Forms
{
  class MITKFORMS_EXPORT ScreenshotQuestion : public Question
  {
  public:
    ~ScreenshotQuestion() override;

    std::string GetType() const override;
    Question* CreateAnother() const override;

    void FromJSON(const nlohmann::ordered_json& j) override;
    void ToJSON(nlohmann::ordered_json& j) const override;

    std::vector<std::string> GetResponsesAsStrings() const override;
    void ClearResponses() override;

    bool IsComplete() const override;

    bool HasFileResponses() const override;
    std::vector<fs::path> SubmitFileResponses(const fs::path& basePath) const override;

    void AddScreenshot(const fs::path& path);
    void RemoveScreenshot(const fs::path& path);

  private:
    std::set<fs::path> m_Responses;
  };

  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, ScreenshotQuestion& q);
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const ScreenshotQuestion& q);
}

#endif
