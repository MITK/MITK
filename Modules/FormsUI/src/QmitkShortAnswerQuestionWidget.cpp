/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkShortAnswerQuestionWidget.h>
#include <mitkExceptionMacro.h>

#include <QLineEdit>
#include <QVBoxLayout>

using namespace mitk::Forms;
using Self = QmitkShortAnswerQuestionWidget;

QmitkShortAnswerQuestionWidget::QmitkShortAnswerQuestionWidget(QWidget* parent)
  : QmitkQuestionWidget(parent),
    m_Question(nullptr),
    m_Layout(new QVBoxLayout),
    m_LineEdit(new QLineEdit)
{
  m_LineEdit->setPlaceholderText("Your answer");
  m_Layout->addWidget(m_LineEdit);

  this->InsertLayout(m_Layout);

  connect(m_LineEdit, &QLineEdit::textEdited, this, &Self::OnTextEdited);
}

QmitkShortAnswerQuestionWidget::~QmitkShortAnswerQuestionWidget()
{
}

QmitkQuestionWidget* QmitkShortAnswerQuestionWidget::CreateAnother(QWidget* parent) const
{
  return new QmitkShortAnswerQuestionWidget(parent);
}

Question* QmitkShortAnswerQuestionWidget::GetQuestion() const
{
  return m_Question;
}

void QmitkShortAnswerQuestionWidget::SetQuestion(Question* question)
{
  QmitkQuestionWidget::SetQuestion(question);

  auto shortAnswerQuestion = dynamic_cast<ShortAnswerQuestion*>(question);

  if (shortAnswerQuestion == nullptr)
    mitkThrow() << "QmitkShortAnswerQuestionWidget only accepts ShortAnswerQuestion as question type!";

  m_Question = shortAnswerQuestion;
}

void QmitkShortAnswerQuestionWidget::Reset()
{
  m_LineEdit->clear();

  if (m_Question)
    m_Question->ClearResponses();

  QmitkQuestionWidget::Reset();
}

void QmitkShortAnswerQuestionWidget::OnTextEdited(const QString& text)
{
  m_Question->SetResponse(text.toStdString());

  if (m_Question->IsRequired())
    this->SetRequirementVisible(!m_Question->IsComplete());
}
