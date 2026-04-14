/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkPipInstallAdvancedDialog.h>

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>

QmitkPipInstallAdvancedDialog::QmitkPipInstallAdvancedDialog(const mitk::PipInstallSpec& spec, QWidget* parent)
  : QDialog(parent),
    m_Spec(spec)
{
  setWindowTitle("Advanced Settings");

  auto* mainLayout = new QVBoxLayout(this);
  bool singleGroup = m_Spec.groups.size() == 1;

  for (int i = 0; i < static_cast<int>(m_Spec.groups.size()); ++i)
  {
    const auto& group = m_Spec.groups[i];

    auto* formLayout = new QFormLayout;

    auto* requirements = new QPlainTextEdit;
    requirements->setTabChangesFocus(true);
    requirements->setMaximumHeight(80);

    QStringList reqLines;
    for (const auto& req : group.requirements)
      reqLines.append(QString::fromStdString(req));
    requirements->setPlainText(reqLines.join('\n'));

    formLayout->addRow("Requirements:", requirements);

    auto* indexUrl = new QLineEdit;
    indexUrl->setPlaceholderText("Default (PyPI)");
    indexUrl->setText(QString::fromStdString(group.indexUrl));
    formLayout->addRow("Index URL:", indexUrl);

    auto* extraPipArgs = new QLineEdit;
    extraPipArgs->setPlaceholderText("e.g. --no-cache-dir");

    QStringList argsList;
    for (const auto& arg : group.extraPipArgs)
      argsList.append(QString::fromStdString(arg));
    extraPipArgs->setText(argsList.join(' '));

    formLayout->addRow("Extra pip arguments:", extraPipArgs);

    m_GroupWidgets.push_back({ requirements, indexUrl, extraPipArgs });

    if (singleGroup)
    {
      mainLayout->addLayout(formLayout);
    }
    else
    {
      auto* groupBox = new QGroupBox(QString("Install Group %1").arg(i + 1));
      groupBox->setLayout(formLayout);
      mainLayout->addWidget(groupBox);
    }
  }

  auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  mainLayout->addWidget(buttonBox);
}

mitk::PipInstallSpec QmitkPipInstallAdvancedDialog::GetInstallSpec() const
{
  auto spec = m_Spec;

  for (int i = 0; i < static_cast<int>(m_GroupWidgets.size()); ++i)
  {
    auto& group = spec.groups[i];
    const auto& widgets = m_GroupWidgets[i];

    group.requirements.clear();
    auto lines = widgets.requirements->toPlainText().split('\n', Qt::SkipEmptyParts);
    for (const auto& line : lines)
    {
      auto trimmed = line.trimmed();
      if (!trimmed.isEmpty())
        group.requirements.push_back(trimmed.toStdString());
    }

    group.indexUrl = widgets.indexUrl->text().trimmed().toStdString();

    group.extraPipArgs.clear();
    auto args = widgets.extraPipArgs->text().split(' ', Qt::SkipEmptyParts);
    for (const auto& arg : args)
    {
      auto trimmed = arg.trimmed();
      if (!trimmed.isEmpty())
        group.extraPipArgs.push_back(trimmed.toStdString());
    }
  }

  return spec;
}
