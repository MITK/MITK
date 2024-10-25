/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkQuestionFactory_h
#define mitkQuestionFactory_h

#include <mitkIQuestionFactory.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace mitk::Forms
{
  class QuestionFactory : public IQuestionFactory
  {
  public:
    QuestionFactory();
    ~QuestionFactory() override;

    void Register(Question* question) override;
    Question* Create(const std::string& type) const override;

  private:
    std::unordered_map<std::string, std::unique_ptr<Question>> m_Prototypes;
  };
}

#endif
