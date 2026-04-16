/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkPipInstallAdvancedDialog.h>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProcess>
#include <QVBoxLayout>

QmitkPipInstallAdvancedDialog::QmitkPipInstallAdvancedDialog(const mitk::PipInstallSpec& spec, QWidget* parent)
  : QDialog(parent),
    m_Spec(spec)
{
  setWindowTitle("Advanced Settings");

  auto* mainLayout = new QVBoxLayout(this);
  const bool hasDownloads = !m_Spec.huggingFaceDownloads.empty();
  const bool inlineGroup = m_Spec.groups.size() == 1 && !hasDownloads;

  for (int i = 0; i < static_cast<int>(m_Spec.groups.size()); ++i)
  {
    const auto& group = m_Spec.groups[i];

    auto* formLayout = new QFormLayout;

    auto* requirements = new QPlainTextEdit;
    requirements->setTabChangesFocus(true);
    requirements->setMinimumHeight(60);
    requirements->setPlaceholderText("One requirement per line (e.g. torch>=2.8.0,<2.9.0)");

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

    if (inlineGroup)
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

  for (int i = 0; i < static_cast<int>(m_Spec.huggingFaceDownloads.size()); ++i)
  {
    const auto& download = m_Spec.huggingFaceDownloads[i];

    auto* formLayout = new QFormLayout;

    auto* repoId = new QLineEdit;
    repoId->setPlaceholderText("e.g. owner/repository");
    repoId->setText(QString::fromStdString(download.repoId));
    formLayout->addRow("Repository ID:", repoId);

    auto* allowPatterns = new QPlainTextEdit;
    allowPatterns->setTabChangesFocus(true);
    allowPatterns->setMinimumHeight(60);
    allowPatterns->setPlaceholderText("One pattern per line, leave empty to download the whole repository");

    QStringList patternLines;
    for (const auto& p : download.allowPatterns)
      patternLines.append(QString::fromStdString(p));
    allowPatterns->setPlainText(patternLines.join('\n'));

    formLayout->addRow("Allow patterns:", allowPatterns);

    auto* optionalCheckBox = new QCheckBox("Optional (failure does not block installation)");
    optionalCheckBox->setChecked(download.optional);
    formLayout->addRow("", optionalCheckBox);

    m_DownloadWidgets.push_back({ repoId, allowPatterns, optionalCheckBox });

    auto* groupBox = new QGroupBox(QString("Model Download %1").arg(i + 1));
    groupBox->setLayout(formLayout);
    mainLayout->addWidget(groupBox);
  }

  m_UpgradePipFirstCheckBox = new QCheckBox("Upgrade pip before installing");
  m_UpgradePipFirstCheckBox->setChecked(m_Spec.upgradePipFirst);
  mainLayout->addWidget(m_UpgradePipFirstCheckBox);

  auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  mainLayout->addWidget(buttonBox);

  // QPlainTextEdit::sizeHint() returns a size for several lines of text, which
  // dominates the dialog's default height. Force the initial height to the
  // layout minimum (honoring each QPlainTextEdit's minimumHeight above). The
  // user can still drag the dialog larger.
  this->resize(this->sizeHint().width(), mainLayout->minimumSize().height());
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

    // Use shell-style tokenization so quoted arguments like
    //   --find-links "D:/my packages"
    // survive a round trip through the dialog.
    group.extraPipArgs.clear();
    const auto args = QProcess::splitCommand(widgets.extraPipArgs->text());
    for (const auto& arg : args)
    {
      if (!arg.isEmpty())
        group.extraPipArgs.push_back(arg.toStdString());
    }
  }

  for (int i = 0; i < static_cast<int>(m_DownloadWidgets.size()); ++i)
  {
    auto& download = spec.huggingFaceDownloads[i];
    const auto& widgets = m_DownloadWidgets[i];

    download.repoId = widgets.repoId->text().trimmed().toStdString();

    download.allowPatterns.clear();
    const auto lines = widgets.allowPatterns->toPlainText().split('\n', Qt::SkipEmptyParts);
    for (const auto& line : lines)
    {
      const auto trimmed = line.trimmed();
      if (!trimmed.isEmpty())
        download.allowPatterns.push_back(trimmed.toStdString());
    }

    download.optional = widgets.optionalCheckBox->isChecked();
  }

  spec.upgradePipFirst = m_UpgradePipFirstCheckBox->isChecked();

  return spec;
}
