/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkQuestionWidget.h>

#include <mitkExceptionMacro.h>
#include <mitkQuestion.h>

#include <QLabel>
#include <QVBoxLayout>

QmitkQuestionWidget::QmitkQuestionWidget(QWidget* parent)
  : QFrame(parent),
    m_Layout(new QVBoxLayout(this)),
    m_QuestionLabel(new QLabel),
    m_RequiredLabel(new QLabel)
{
  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
  this->setFrameShape(QFrame::Box);

  m_Layout->addWidget(m_QuestionLabel);
  m_Layout->addWidget(m_RequiredLabel);

  m_QuestionLabel->setWordWrap(true);

  m_RequiredLabel->setWordWrap(true);
  m_RequiredLabel->hide();
}

QmitkQuestionWidget::~QmitkQuestionWidget()
{
}

void QmitkQuestionWidget::SetQuestion(mitk::Forms::Question* question)
{
  if (question == nullptr)
    mitkThrow() << "Invalid question: cannot be nullptr!";

  auto text = QString::fromStdString(question->GetQuestionText());

  if (question->IsRequired())
  {
    if (!text.isEmpty())
      text.append(' ');

    text.append("<span style=\"color: red;\">*</span>");
  }

  m_QuestionLabel->setText(QString("<h4>%1</h4>").arg(text));
  m_RequiredLabel->setText(QString("<span style=\"color: red;\">%1</span>").arg(QString::fromStdString(question->GetRequiredText())));
}

void QmitkQuestionWidget::Reset()
{
  this->HideRequirement();
}

void QmitkQuestionWidget::SetRequirementVisible(bool visible)
{
  if (visible)
  {
    this->ShowRequirement();
  }
  else
  {
    this->HideRequirement();
  }
}

void QmitkQuestionWidget::ShowRequirement()
{
  this->setStyleSheet("QmitkQuestionWidget { border-color: red; }");
  m_RequiredLabel->show();
}

void QmitkQuestionWidget::HideRequirement()
{
  this->setStyleSheet("");
  m_RequiredLabel->hide();
}

void QmitkQuestionWidget::InsertLayout(QLayout* layout)
{
  m_Layout->insertLayout(1, layout);
}
