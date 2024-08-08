/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTextQuestion_h
#define mitkTextQuestion_h

#include <mitkQuestion.h>

namespace mitk::Forms
{
  class MITKFORMS_EXPORT TextQuestion : public Question
  {
  public:
    ~TextQuestion() override;

    std::vector<std::string> GetResponsesAsStrings() const override;
    void ClearResponses() override;

    bool IsComplete() const override;

    std::string GetResponse() const;
    void SetResponse(const std::string& response);

  private:
    std::string m_Response;
  };
}

#endif
