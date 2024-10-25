/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMultipleChoiceQuestionWidget.h>
#include <mitkExceptionMacro.h>

#include <QButtonGroup>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>

using namespace mitk::Forms;
using Self = QmitkMultipleChoiceQuestionWidget;

QmitkMultipleChoiceQuestionWidget::QmitkMultipleChoiceQuestionWidget(QWidget* parent)
  : QmitkQuestionWidget(parent),
    m_Question(nullptr),
    m_Layout(new QGridLayout),
    m_ButtonGroup(nullptr),
    m_OtherLineEdit(nullptr),
    m_OtherId(-1),
    m_ClearButton(nullptr)
{
  this->InsertLayout(m_Layout);
}

QmitkMultipleChoiceQuestionWidget::~QmitkMultipleChoiceQuestionWidget()
{
}

QmitkQuestionWidget* QmitkMultipleChoiceQuestionWidget::CreateAnother(QWidget* parent) const
{
  return new QmitkMultipleChoiceQuestionWidget(parent);
}

Question* QmitkMultipleChoiceQuestionWidget::GetQuestion() const
{
  return m_Question;
}

void QmitkMultipleChoiceQuestionWidget::SetQuestion(Question* question)
{
  QmitkQuestionWidget::SetQuestion(question);

  auto multipleChoiceQuestion = dynamic_cast<MultipleChoiceQuestion*>(question);

  if (multipleChoiceQuestion == nullptr)
    mitkThrow() << "QmitkMultipleChoiceQuestionWidget only accepts MultipleChoiceQuestion as question type!";

  m_Question = multipleChoiceQuestion;

  this->CreateWidgets();
}

void QmitkMultipleChoiceQuestionWidget::Reset()
{
  this->OnClearButtonClicked();

  if (m_OtherLineEdit != nullptr)
    m_OtherLineEdit->clear();

  if (m_Question != nullptr)
    m_Question->ClearResponses();

  QmitkQuestionWidget::Reset();
}

void QmitkMultipleChoiceQuestionWidget::CreateWidgets()
{
  this->RemoveWidgets();

  int row = 0;

  for (const auto& option : m_Question->GetOptions())
  {
    auto optionRadioButton = new QRadioButton(QString::fromStdString(option));
    m_Layout->addWidget(optionRadioButton, row, 0, 1, 2);
    m_ButtonGroup->addButton(optionRadioButton, row++);
  }

  if (m_Question->HasOtherOption())
  {
    auto otherRadioButton = new QRadioButton("Other:");
    m_Layout->addWidget(otherRadioButton, row, 0);
    m_ButtonGroup->addButton(otherRadioButton, row);
    m_OtherId = row;

    m_OtherLineEdit = new QLineEdit;
    m_Layout->addWidget(m_OtherLineEdit, row++, 1);

    connect(m_OtherLineEdit, &QLineEdit::textEdited, this, &Self::OnTextEdited);
    connect(m_OtherLineEdit, &QLineEdit::editingFinished, this, &Self::OnEditingFinished);
  }
  else
  {
    m_OtherId = -1;
  }

  if (m_Question->IsRequired())
    return;

  m_ClearButton = new QPushButton("Clear selection");
  m_ClearButton->setFlat(true);
  m_ClearButton->hide();

  m_Layout->addWidget(m_ClearButton, row, 0, 1, 2, Qt::AlignRight);

  connect(m_ClearButton, &QPushButton::clicked, this, &Self::OnClearButtonClicked);
}

void QmitkMultipleChoiceQuestionWidget::RemoveWidgets()
{
  if (m_ButtonGroup != nullptr)
    delete m_ButtonGroup;

  m_ButtonGroup = new QButtonGroup(this);

  connect(m_ButtonGroup, &QButtonGroup::idClicked, this, &Self::OnIdClicked);

  QLayoutItem* child;
  while ((child = m_Layout->takeAt(0)) != nullptr)
  {
    delete child->widget();
    delete child;
  }

  m_OtherLineEdit = nullptr;
  m_ClearButton = nullptr;
}

void QmitkMultipleChoiceQuestionWidget::OnIdClicked(int id)
{
  if (id == m_OtherId)
  {
    m_Question->SetOtherResponse(m_OtherLineEdit->text().toStdString());
    m_OtherLineEdit->setFocus();
  }
  else
  {
    m_Question->SetResponse(static_cast<size_t>(id));
  }

  if (m_ClearButton != nullptr)
    m_ClearButton->show();

  if (m_Question->IsRequired())
    this->SetRequirementVisible(!m_Question->IsComplete());
}

void QmitkMultipleChoiceQuestionWidget::OnTextEdited(const QString&)
{
  if (m_ButtonGroup->checkedId() != m_OtherId)
    m_ButtonGroup->button(m_OtherId)->click();
}

void QmitkMultipleChoiceQuestionWidget::OnEditingFinished()
{
  if (m_ButtonGroup->checkedId() == m_OtherId)
    m_Question->SetOtherResponse(m_OtherLineEdit->text().toStdString());
}

void QmitkMultipleChoiceQuestionWidget::OnClearButtonClicked()
{
  auto checkedButton = m_ButtonGroup->checkedButton();

  if (checkedButton != nullptr)
  {
    m_ButtonGroup->setExclusive(false);
    checkedButton->setChecked(false);
    m_ButtonGroup->setExclusive(true);
  }

  if (m_ClearButton != nullptr)
    m_ClearButton->hide();
}
