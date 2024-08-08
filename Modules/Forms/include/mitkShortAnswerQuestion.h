/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkShortAnswerQuestion_h
#define mitkShortAnswerQuestion_h

#include <mitkTextQuestion.h>

namespace mitk::Forms
{
  class MITKFORMS_EXPORT ShortAnswerQuestion : public TextQuestion
  {
  public:
    ~ShortAnswerQuestion() override;

    std::string GetType() const override;
    Question* CreateAnother() const override;

    void FromJSON(const nlohmann::ordered_json& j) override;
    void ToJSON(nlohmann::ordered_json& j) const override;
  };

  MITKFORMS_EXPORT void from_json(const nlohmann::ordered_json& j, ShortAnswerQuestion& q);
  MITKFORMS_EXPORT void to_json(nlohmann::ordered_json& j, const ShortAnswerQuestion& q);
}

#endif
