/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkQuestionWidgetFactory_h
#define mitkQuestionWidgetFactory_h

#include <mitkIQuestionWidgetFactory.h>

#include <memory>
#include <unordered_map>

namespace mitk::Forms::UI
{
  class QuestionWidgetFactory : public IQuestionWidgetFactory
  {
  public:
    QuestionWidgetFactory();
    ~QuestionWidgetFactory() override;

    void Register(const std::string& questionType, QmitkQuestionWidget* widgetPrototype) override;
    QmitkQuestionWidget* Create(Question* question, QWidget* parent = nullptr) const override;

  private:
    std::unordered_map<std::string, std::unique_ptr<QmitkQuestionWidget>> m_WidgetPrototypes;
  };
}

#endif
