/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkDropdownQuestionWidget.h>
#include <mitkExceptionMacro.h>

#include <QVBoxLayout>
#include <QComboBox>

using namespace mitk::Forms;
using Self = QmitkDropdownQuestionWidget;

QmitkDropdownQuestionWidget::QmitkDropdownQuestionWidget(QWidget* parent)
  : QmitkQuestionWidget(parent),
    m_Question(nullptr),
    m_Layout(new QVBoxLayout),
    m_ComboBox(new QComboBox)
{
  m_Layout->addWidget(m_ComboBox);
  
  this->InsertLayout(m_Layout);

  connect(m_ComboBox, &QComboBox::currentIndexChanged, this, &Self::OnCurrentIndexChanged);
}

QmitkDropdownQuestionWidget::~QmitkDropdownQuestionWidget()
{
}

QmitkQuestionWidget* QmitkDropdownQuestionWidget::CreateAnother(QWidget* parent) const
{
  return new QmitkDropdownQuestionWidget(parent);
}

Question* QmitkDropdownQuestionWidget::GetQuestion() const
{
  return m_Question;
}

void QmitkDropdownQuestionWidget::SetQuestion(Question* question)
{
  QmitkQuestionWidget::SetQuestion(question);

  auto dropdownQuestion = dynamic_cast<DropdownQuestion*>(question);

  if (dropdownQuestion == nullptr)
    mitkThrow() << "QmitkDropdownQuestionWidget only accepts DropdownQuestion as question type!";

  m_Question = dropdownQuestion;

  this->CreateWidgets();
}

void QmitkDropdownQuestionWidget::Reset()
{
  m_ComboBox->setCurrentIndex(0);

  if (m_Question)
    m_Question->ClearResponses();

  QmitkQuestionWidget::Reset();
}

void QmitkDropdownQuestionWidget::CreateWidgets()
{
  this->RemoveWidgets();

  for (const auto& option : m_Question->GetOptions())
    m_ComboBox->addItem(QString::fromStdString(option));
}

void QmitkDropdownQuestionWidget::RemoveWidgets()
{
  m_ComboBox->clear();
  m_ComboBox->addItem("Choose");
  m_ComboBox->insertSeparator(1);
}

void QmitkDropdownQuestionWidget::OnCurrentIndexChanged(int index)
{
  if (m_Question == nullptr)
    return;

  if (index > 1)
  {
    m_Question->SetResponse(static_cast<size_t>(index - 2));
  }
  else
  {
    m_Question->ClearResponses();
  }

  if (m_Question->IsRequired())
    this->SetRequirementVisible(!m_Question->IsComplete());
}
