/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkCheckboxesQuestionWidget.h>
#include <mitkExceptionMacro.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QLineEdit>

using namespace mitk::Forms;
using Self = QmitkCheckboxesQuestionWidget;

QmitkCheckboxesQuestionWidget::QmitkCheckboxesQuestionWidget(QWidget* parent)
  : QmitkQuestionWidget(parent),
    m_Question(nullptr),
    m_Layout(new QGridLayout),
    m_ButtonGroup(nullptr),
    m_OtherLineEdit(nullptr),
    m_OtherId(-1)
{
  this->InsertLayout(m_Layout);
}

QmitkCheckboxesQuestionWidget::~QmitkCheckboxesQuestionWidget()
{
}

QmitkQuestionWidget* QmitkCheckboxesQuestionWidget::CreateAnother(QWidget* parent) const
{
  return new QmitkCheckboxesQuestionWidget(parent);
}

Question* QmitkCheckboxesQuestionWidget::GetQuestion() const
{
  return m_Question;
}

void QmitkCheckboxesQuestionWidget::SetQuestion(Question* question)
{
  QmitkQuestionWidget::SetQuestion(question);

  auto checkboxesQuestion = dynamic_cast<CheckboxesQuestion*>(question);

  if (checkboxesQuestion == nullptr)
    mitkThrow() << "QmitkCheckboxesQuestionWidget only accepts CheckboxesQuestion as question type!";

  m_Question = checkboxesQuestion;

  this->CreateWidgets();
}

void QmitkCheckboxesQuestionWidget::Reset()
{
  if (m_ButtonGroup != nullptr)
  {
    for (auto checkBox : m_ButtonGroup->buttons())
      checkBox->setChecked(false);
  }

  if (m_OtherLineEdit != nullptr)
    m_OtherLineEdit->clear();

  if (m_Question != nullptr)
    m_Question->ClearResponses();

  QmitkQuestionWidget::Reset();
}

void QmitkCheckboxesQuestionWidget::CreateWidgets()
{
  this->RemoveWidgets();

  int row = 0;

  for (const auto& option : m_Question->GetOptions())
  {
    auto checkBox = new QCheckBox(QString::fromStdString(option));
    m_Layout->addWidget(checkBox, row, 0, 1, 2);
    m_ButtonGroup->addButton(checkBox, row++);
  }

  if (m_Question->HasOtherOption())
  {
    auto checkBox = new QCheckBox("Other:");
    m_Layout->addWidget(checkBox, row, 0);
    m_ButtonGroup->addButton(checkBox, row);
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
}

void QmitkCheckboxesQuestionWidget::RemoveWidgets()
{
  if (m_ButtonGroup != nullptr)
    delete m_ButtonGroup;

  m_ButtonGroup = new QButtonGroup(this);
  m_ButtonGroup->setExclusive(false);

  connect(m_ButtonGroup, &QButtonGroup::idToggled, this, &Self::OnIdToggled);

  QLayoutItem* child;
  while ((child = m_Layout->takeAt(0)) != nullptr)
  {
    delete child->widget();
    delete child;
  }

  m_OtherLineEdit = nullptr;
}

void QmitkCheckboxesQuestionWidget::OnIdToggled(int id, bool checked)
{
  if (checked)
  {
    if (id == m_OtherId)
    {
      m_Question->AddOtherResponse(m_OtherLineEdit->text().toStdString());
      m_OtherLineEdit->setFocus();
    }
    else
    {
      m_Question->AddResponse(static_cast<size_t>(id));
    }
  }
  else
  {
    if (id == m_OtherId)
    {
      m_Question->RemoveOtherResponse();
    }
    else
    {
      m_Question->RemoveResponse(static_cast<size_t>(id));
    }
  }

  if (m_Question->IsRequired())
    this->SetRequirementVisible(!m_Question->IsComplete());
}

void QmitkCheckboxesQuestionWidget::OnTextEdited(const QString&)
{
  if (!m_ButtonGroup->button(m_OtherId)->isChecked())
    m_ButtonGroup->button(m_OtherId)->click();
}

void QmitkCheckboxesQuestionWidget::OnEditingFinished()
{
  if (m_ButtonGroup->button(m_OtherId)->isChecked())
    m_Question->AddOtherResponse(m_OtherLineEdit->text().toStdString());
}
