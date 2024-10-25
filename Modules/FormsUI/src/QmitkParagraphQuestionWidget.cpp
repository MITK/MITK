/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkParagraphQuestionWidget.h>
#include <mitkExceptionMacro.h>

#include <QAbstractTextDocumentLayout>
#include <QTextEdit>
#include <QVBoxLayout>

using namespace mitk::Forms;
using Self = QmitkParagraphQuestionWidget;

QmitkParagraphQuestionWidget::QmitkParagraphQuestionWidget(QWidget* parent)
  : QmitkQuestionWidget(parent),
    m_Question(nullptr),
    m_Layout(new QVBoxLayout),
    m_TextEdit(new QTextEdit)
{
  m_TextEdit->setPlaceholderText("Your answer");
  m_Layout->addWidget(m_TextEdit);

  this->InsertLayout(m_Layout);
  this->AdjustHeight();

  connect(m_TextEdit, &QTextEdit::textChanged, this, &Self::OnTextChanged);
}

QmitkParagraphQuestionWidget::~QmitkParagraphQuestionWidget()
{
}

QmitkQuestionWidget* QmitkParagraphQuestionWidget::CreateAnother(QWidget* parent) const
{
  return new QmitkParagraphQuestionWidget(parent);
}

Question* QmitkParagraphQuestionWidget::GetQuestion() const
{
  return m_Question;
}

void QmitkParagraphQuestionWidget::SetQuestion(Question* question)
{
  QmitkQuestionWidget::SetQuestion(question);

  auto paragraphQuestion = dynamic_cast<ParagraphQuestion*>(question);

  if (paragraphQuestion == nullptr)
    mitkThrow() << "QmitkParagraphQuestionWidget only accepts ParagraphQuestion as question type!";

  m_Question = paragraphQuestion;
}

void QmitkParagraphQuestionWidget::Reset()
{
  m_TextEdit->clear();

  if (m_Question)
    m_Question->ClearResponses();

  QmitkQuestionWidget::Reset();
}

void QmitkParagraphQuestionWidget::AdjustHeight()
{
  QMargins contentsMargins = m_TextEdit->contentsMargins();
  int documentHeight = static_cast<int>(m_TextEdit->document()->documentLayout()->documentSize().height());
  int totalHeight = contentsMargins.top() + documentHeight + contentsMargins.bottom();

  m_TextEdit->setFixedHeight(std::max(totalHeight, 50));
}

void QmitkParagraphQuestionWidget::OnTextChanged()
{
  m_Question->SetResponse(m_TextEdit->toPlainText().toStdString());

  if (m_Question->IsRequired())
    this->SetRequirementVisible(!m_Question->IsComplete());

  this->AdjustHeight();
}
