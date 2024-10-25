/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkLinearScaleQuestionWidget.h>
#include <mitkExceptionMacro.h>

#include <QButtonGroup>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>

using namespace mitk::Forms;
using Self = QmitkLinearScaleQuestionWidget;

QmitkLinearScaleQuestionWidget::QmitkLinearScaleQuestionWidget(QWidget* parent)
  : QmitkQuestionWidget(parent),
    m_Question(nullptr),
    m_Layout(new QGridLayout),
    m_ButtonGroup(nullptr),
    m_ClearButton(nullptr)
{
  this->InsertLayout(m_Layout);
}

QmitkLinearScaleQuestionWidget::~QmitkLinearScaleQuestionWidget()
{
}

QmitkQuestionWidget* QmitkLinearScaleQuestionWidget::CreateAnother(QWidget* parent) const
{
  return new QmitkLinearScaleQuestionWidget(parent);
}

Question* QmitkLinearScaleQuestionWidget::GetQuestion() const
{
  return m_Question;
}

void QmitkLinearScaleQuestionWidget::SetQuestion(Question* question)
{
  QmitkQuestionWidget::SetQuestion(question);

  auto linearScaleQuestion = dynamic_cast<LinearScaleQuestion*>(question);

  if (linearScaleQuestion == nullptr)
    mitkThrow() << "QmitkLinearScaleQuestionWidget only accepts LinearScaleQuestion as question type!";

  m_Question = linearScaleQuestion;

  this->CreateWidgets();
}

void QmitkLinearScaleQuestionWidget::Reset()
{
  this->OnClearButtonClicked();

  if (m_Question)
    m_Question->ClearResponses();

  QmitkQuestionWidget::Reset();
}

void QmitkLinearScaleQuestionWidget::CreateWidgets()
{
  this->RemoveWidgets();

  auto [from, to] = m_Question->GetRange();
  auto [fromText, toText] = m_Question->GetRangeLabels();
  int column = 0;

  if (!fromText.empty())
  {
    auto fromLabel = new QLabel(QString::fromStdString(fromText));
    m_Layout->addWidget(fromLabel, 1, column++, Qt::AlignRight);
  }

  for (int i = from; i <= to; ++i)
  {
    auto label = new QLabel(QString("%1").arg(i));

    auto radioButton = new QRadioButton;
    radioButton->setFixedWidth(radioButton->sizeHint().height());

    m_Layout->addWidget(label, 0, column, Qt::AlignHCenter);
    m_Layout->addWidget(radioButton, 1, column++, Qt::AlignHCenter);

    m_ButtonGroup->addButton(radioButton, i);
  }

  if (!toText.empty())
  {
    auto toLabel = new QLabel(QString::fromStdString(toText));
    m_Layout->addWidget(toLabel, 1, column++, Qt::AlignLeft);
  }

  if (m_Question->IsRequired())
    return;

  m_ClearButton = new QPushButton("Clear selection");
  m_ClearButton->setFlat(true);
  m_ClearButton->hide();

  m_Layout->addWidget(m_ClearButton, 2, 0, 1, column, Qt::AlignRight);

  connect(m_ClearButton, &QPushButton::clicked, this, &Self::OnClearButtonClicked);
}

void QmitkLinearScaleQuestionWidget::RemoveWidgets()
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

  m_ClearButton = nullptr;
}

void QmitkLinearScaleQuestionWidget::OnIdClicked(int id)
{
  m_Question->SetResponse(id);

  if (m_ClearButton != nullptr)
    m_ClearButton->show();

  if (m_Question->IsRequired())
    this->SetRequirementVisible(!m_Question->IsComplete());
}

void QmitkLinearScaleQuestionWidget::OnClearButtonClicked()
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
