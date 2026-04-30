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
  /**
   * \brief Default implementation of IQuestionWidgetFactory using prototype-based creation.
   *
   * Stores widget prototypes keyed by question type string and creates
   * new widget instances by cloning the matching prototype.
   */
  class QuestionWidgetFactory : public IQuestionWidgetFactory
  {
  public:
    QuestionWidgetFactory();
    ~QuestionWidgetFactory() override;

    /**
     * \brief Register a widget prototype for a given question type. Takes ownership.
     * \param questionType The question type string this widget handles.
     * \param widgetPrototype The prototype widget to register.
     */
    void Register(const std::string& questionType, QmitkQuestionWidget* widgetPrototype) override;

    /**
     * \brief Create a new widget instance for the given question.
     * \param question The question to create a widget for.
     * \param parent Optional parent widget.
     * \return A new QmitkQuestionWidget, or nullptr if no prototype matches.
     */
    QmitkQuestionWidget* Create(Question* question, QWidget* parent = nullptr) const override;

  private:
    std::unordered_map<std::string, std::unique_ptr<QmitkQuestionWidget>> m_WidgetPrototypes;
  };
}

#endif
