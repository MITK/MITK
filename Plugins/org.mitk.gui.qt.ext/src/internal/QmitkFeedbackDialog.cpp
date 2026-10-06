/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFeedbackDialog.h"

#include <ui_QmitkFeedbackDialog.h>

#include <mitkFeedbackBundle.h>

#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

QmitkFeedbackDialog::QmitkFeedbackDialog(const mitk::EnvironmentInfo& environment, QWidget* parent)
  : QDialog(parent),
    m_Controls(std::make_unique<Ui::QmitkFeedbackDialog>())
{
  m_Controls->setupUi(this);
  m_Controls->splitter->setStretchFactor(0, 3);
  m_Controls->splitter->setStretchFactor(1, 2);

  for (const auto category : { mitk::FeedbackCategory::Bug, mitk::FeedbackCategory::Feature, mitk::FeedbackCategory::Idea })
    m_Controls->categoryComboBox->addItem(QString::fromStdString(mitk::ToString(category)), static_cast<int>(category));

  this->AddEnvironmentItems(environment);

  connect(m_Controls->categoryComboBox, &QComboBox::currentIndexChanged, this, &QmitkFeedbackDialog::UpdatePreview);
  connect(m_Controls->descriptionEdit, &QPlainTextEdit::textChanged, this, &QmitkFeedbackDialog::UpdatePreview);
  connect(m_Controls->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  this->UpdatePreview();
}

QmitkFeedbackDialog::~QmitkFeedbackDialog() = default;

void QmitkFeedbackDialog::AddEnvironmentItems(const mitk::EnvironmentInfo& environment)
{
  auto* layout = m_Controls->environmentLayout;
  layout->setColumnStretch(1, 1);

  int row = 0;

  for (const auto& [label, value] : mitk::ToLabeledFields(environment))
  {
    auto* checkBox = new QCheckBox(QString::fromStdString(label));
    checkBox->setChecked(true);

    auto* valueLabel = new QLabel(QString::fromStdString(value));
    valueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    valueLabel->setWordWrap(true);

    layout->addWidget(checkBox, row, 0, Qt::AlignTop);
    layout->addWidget(valueLabel, row, 1);
    ++row;

    m_EnvironmentItems.emplace_back(checkBox, mitk::FeedbackItem{ label, value, true });
    connect(checkBox, &QCheckBox::toggled, this, &QmitkFeedbackDialog::UpdatePreview);
  }
}

mitk::FeedbackReport QmitkFeedbackDialog::AssembleReport() const
{
  mitk::FeedbackReport report;
  report.category = static_cast<mitk::FeedbackCategory>(m_Controls->categoryComboBox->currentData().toInt());
  report.description = m_Controls->descriptionEdit->toPlainText().trimmed().toStdString();

  for (const auto& [checkBox, item] : m_EnvironmentItems)
  {
    report.environment.push_back(item);
    report.environment.back().included = checkBox->isChecked();
  }

  return report;
}

void QmitkFeedbackDialog::UpdatePreview()
{
  m_Controls->previewEdit->setPlainText(QString::fromStdString(mitk::BuildReportText(this->AssembleReport())));
}
