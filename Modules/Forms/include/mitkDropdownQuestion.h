/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDropdownQuestion_h
#define mitkDropdownQuestion_h

#include <mitkQuestionWithOptions.h>

namespace mitk::Forms
{
  class MITKFORMS_EXPORT DropdownQuestion : public QuestionWithOptions
  {
  public:
    ~DropdownQuestion() override;

    std::string GetType() const override;
    Question* CreateAnother() const override;

    void FromJSON(const nlohmann::ordered_json& j) override;
    void ToJSON(nlohmann::ordered_json& j) const override;

    using QuestionWithOptions::SetResponse;
  };

  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, DropdownQuestion& q);
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const DropdownQuestion& q);
}

#endif
