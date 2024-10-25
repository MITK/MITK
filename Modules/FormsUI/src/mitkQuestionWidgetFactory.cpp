/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkQuestionWidgetFactory.h"

#include <mitkExceptionMacro.h>
#include <mitkQuestion.h>

#include <QmitkQuestionWidget.h>

using namespace mitk::Forms::UI;

QuestionWidgetFactory::QuestionWidgetFactory() = default;

QuestionWidgetFactory::~QuestionWidgetFactory()
{
}

void QuestionWidgetFactory::Register(const std::string& questionType, QmitkQuestionWidget* widgetPrototype)
{
  if (widgetPrototype == nullptr)
    mitkThrow() << "Cannot register nullptr as widget prototype!";

  if (m_WidgetPrototypes.find(questionType) != m_WidgetPrototypes.end())
    mitkThrow() << "A widget prototype is already registered for questions of type \"" << questionType << "\"!";

  m_WidgetPrototypes[questionType].reset(widgetPrototype);
}

QmitkQuestionWidget* QuestionWidgetFactory::Create(Question* question, QWidget* parent) const
{
  try
  {
    auto widget = m_WidgetPrototypes.at(question->GetType())->CreateAnother(parent);
    widget->SetQuestion(question);

    return widget;
  }
  catch (const std::out_of_range&)
  {
  }

  return nullptr;
}
