/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLinearScaleQuestion_h
#define mitkLinearScaleQuestion_h

#include <mitkQuestion.h>
#include <optional>
#include <utility>

namespace mitk::Forms
{
  class MITKFORMS_EXPORT LinearScaleQuestion : public Question
  {
  public:
    LinearScaleQuestion();
    ~LinearScaleQuestion() override;

    std::string GetType() const override;
    Question* CreateAnother() const override;

    void FromJSON(const nlohmann::ordered_json& j) override;
    void ToJSON(nlohmann::ordered_json& j) const override;

    std::vector<std::string> GetResponsesAsStrings() const override;
    void ClearResponses() override;

    bool IsComplete() const override;

    std::optional<int> GetResponse() const;
    void SetResponse(int response);

    std::pair<int, int> GetRange() const;
    void SetRange(const std::pair<int, int>& range);

    std::pair<std::string, std::string> GetRangeLabels() const;
    void SetRangeLabels(const std::pair<std::string, std::string>& labels);

  private:
    std::optional<int> m_Response;
    std::pair<int, int> m_Range;
    std::pair<std::string, std::string> m_Labels;
  };

  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, LinearScaleQuestion& q);
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const LinearScaleQuestion& q);
}

#endif
