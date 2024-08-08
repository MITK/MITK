/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkQuestion_h
#define mitkQuestion_h

#include <MitkFormsExports.h>

#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace mitk::Forms
{
  class MITKFORMS_EXPORT Question
  {
  public:
    Question();
    virtual ~Question();

    std::string GetQuestionText() const;
    void SetQuestionText(const std::string& question);

    virtual std::string GetType() const = 0;
    virtual Question* CreateAnother() const = 0;

    virtual void FromJSON(const nlohmann::ordered_json& j) = 0;
    virtual void ToJSON(nlohmann::ordered_json& j) const = 0;

    virtual std::vector<std::string> GetResponsesAsStrings() const = 0;
    virtual void ClearResponses() = 0;

    virtual std::string GetRequiredText() const;
    virtual bool IsComplete() const = 0;

    void SetRequired(bool required = true);
    bool IsRequired() const;

  private:
    std::string m_QuestionText;
    bool m_IsRequired;
  };

  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, Question& q);
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const Question& q);
}

#endif
