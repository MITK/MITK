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
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

QmitkPipInstallAdvancedDialog::QmitkPipInstallAdvancedDialog(const mitk::PipInstallSpec& spec, QWidget* parent)
  : QDialog(parent)
{
  setWindowTitle("Advanced Settings");

  auto* outerLayout = new QVBoxLayout(this);

  // --- Scroll area ---

  auto* scrollArea = new QScrollArea;
  scrollArea->setWidgetResizable(true);
  scrollArea->setFrameShape(QFrame::NoFrame);
  outerLayout->addWidget(scrollArea, 1);

  auto* scrollWidget = new QWidget;
  auto* mainLayout = new QVBoxLayout(scrollWidget);
  scrollArea->setWidget(scrollWidget);

  // --- General section ---

  auto* generalBox = new QGroupBox("General");
  auto* generalForm = new QFormLayout(generalBox);

  m_NameEdit = new QLineEdit;
  m_NameEdit->setPlaceholderText("e.g. nnInteractive");
  generalForm->addRow("Name:", m_NameEdit);

  m_VenvNameEdit = new QLineEdit;
  m_VenvNameEdit->setPlaceholderText("Leave empty to install into the system Python");
  generalForm->addRow("Virtual environment:", m_VenvNameEdit);

  m_UpgradePipFirstCheckBox = new QCheckBox("Upgrade pip before installing");
  generalForm->addRow("", m_UpgradePipFirstCheckBox);

  mainLayout->addWidget(generalBox);

  // --- Install Groups section ---

  mainLayout->addWidget(new QLabel("<b>Install Groups</b>"));

  m_GroupsLayout = new QVBoxLayout;
  mainLayout->addLayout(m_GroupsLayout);

  auto* addGroupButton = new QPushButton("Add Install Group");
  mainLayout->addWidget(addGroupButton);
  connect(addGroupButton, &QPushButton::clicked, this, &QmitkPipInstallAdvancedDialog::OnAddGroup);

  // --- Hugging Face Downloads section ---

  mainLayout->addWidget(new QLabel("<b>Hugging Face Downloads</b>"));

  m_DownloadsLayout = new QVBoxLayout;
  mainLayout->addLayout(m_DownloadsLayout);

  auto* addDownloadButton = new QPushButton("Add Hugging Face Download");
  mainLayout->addWidget(addDownloadButton);
  connect(addDownloadButton, &QPushButton::clicked, this, &QmitkPipInstallAdvancedDialog::OnAddDownload);

  mainLayout->addStretch();

  // --- Bottom row: Load/Save on the left, Ok/Cancel on the right ---

  auto* bottomLayout = new QHBoxLayout;

  auto* loadButton = new QPushButton("Load...");
  auto* saveButton = new QPushButton("Save...");
  connect(loadButton, &QPushButton::clicked, this, &QmitkPipInstallAdvancedDialog::OnLoadSpec);
  connect(saveButton, &QPushButton::clicked, this, &QmitkPipInstallAdvancedDialog::OnSaveSpec);
  bottomLayout->addWidget(loadButton);
  bottomLayout->addWidget(saveButton);

  bottomLayout->addStretch();

  auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  bottomLayout->addWidget(buttonBox);

  outerLayout->addLayout(bottomLayout);

  // Populate from spec.
  this->RebuildFromSpec(spec);

  resize(480, 500);
}

mitk::PipInstallSpec QmitkPipInstallAdvancedDialog::GetInstallSpec() const
{
  mitk::PipInstallSpec spec;

  spec.name = m_NameEdit->text().trimmed().toStdString();
  spec.venvName = m_VenvNameEdit->text().trimmed().toStdString();
  spec.upgradePipFirst = m_UpgradePipFirstCheckBox->isChecked();

  for (const auto& widgets : m_GroupWidgets)
  {
    mitk::PipInstallGroup group;

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
    const auto args = QProcess::splitCommand(widgets.extraPipArgs->text());
    for (const auto& arg : args)
    {
      if (!arg.isEmpty())
        group.extraPipArgs.push_back(arg.toStdString());
    }

    spec.groups.push_back(std::move(group));
  }

  for (const auto& widgets : m_DownloadWidgets)
  {
    mitk::HuggingFaceDownload download;

    download.displayName = widgets.displayName->text().trimmed().toStdString();
    download.repoId = widgets.repoId->text().trimmed().toStdString();

    auto lines = widgets.allowPatterns->toPlainText().split('\n', Qt::SkipEmptyParts);
    for (const auto& line : lines)
    {
      const auto trimmed = line.trimmed();
      if (!trimmed.isEmpty())
        download.allowPatterns.push_back(trimmed.toStdString());
    }

    download.optional = widgets.optionalCheckBox->isChecked();

    spec.huggingFaceDownloads.push_back(std::move(download));
  }

  return spec;
}

QGroupBox* QmitkPipInstallAdvancedDialog::CreateGroupWidget(const mitk::PipInstallGroup& group, int index)
{
  auto* groupBox = new QGroupBox(QString("Install Group %1").arg(index + 1));
  auto* layout = new QVBoxLayout(groupBox);
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

  layout->addLayout(formLayout);

  auto* removeButton = new QPushButton("Remove");
  layout->addWidget(removeButton, 0, Qt::AlignRight);
  connect(removeButton, &QPushButton::clicked, this, &QmitkPipInstallAdvancedDialog::OnRemoveGroup);

  m_GroupWidgets.push_back({ requirements, indexUrl, extraPipArgs });
  m_GroupBoxes.push_back(groupBox);

  return groupBox;
}

QGroupBox* QmitkPipInstallAdvancedDialog::CreateDownloadWidget(const mitk::HuggingFaceDownload& download, int index)
{
  auto* groupBox = new QGroupBox(QString("Hugging Face Download %1").arg(index + 1));
  auto* layout = new QVBoxLayout(groupBox);
  auto* formLayout = new QFormLayout;

  auto* displayName = new QLineEdit;
  displayName->setPlaceholderText("Optional label (falls back to repository ID)");
  displayName->setText(QString::fromStdString(download.displayName));
  formLayout->addRow("Display name:", displayName);

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

  layout->addLayout(formLayout);

  auto* removeButton = new QPushButton("Remove");
  layout->addWidget(removeButton, 0, Qt::AlignRight);
  connect(removeButton, &QPushButton::clicked, this, &QmitkPipInstallAdvancedDialog::OnRemoveDownload);

  m_DownloadWidgets.push_back({ displayName, repoId, allowPatterns, optionalCheckBox });
  m_DownloadBoxes.push_back(groupBox);

  return groupBox;
}

void QmitkPipInstallAdvancedDialog::RenumberGroupTitles()
{
  for (int i = 0; i < static_cast<int>(m_GroupBoxes.size()); ++i)
    m_GroupBoxes[i]->setTitle(QString("Install Group %1").arg(i + 1));
}

void QmitkPipInstallAdvancedDialog::RenumberDownloadTitles()
{
  for (int i = 0; i < static_cast<int>(m_DownloadBoxes.size()); ++i)
    m_DownloadBoxes[i]->setTitle(QString("Hugging Face Download %1").arg(i + 1));
}

void QmitkPipInstallAdvancedDialog::RebuildFromSpec(const mitk::PipInstallSpec& spec)
{
  // Clear existing groups.
  for (auto* box : m_GroupBoxes)
    delete box;

  m_GroupWidgets.clear();
  m_GroupBoxes.clear();

  // Clear existing downloads.
  for (auto* box : m_DownloadBoxes)
    delete box;

  m_DownloadWidgets.clear();
  m_DownloadBoxes.clear();

  // Populate general fields.
  m_NameEdit->setText(QString::fromStdString(spec.name));
  m_VenvNameEdit->setText(QString::fromStdString(spec.venvName));
  m_UpgradePipFirstCheckBox->setChecked(spec.upgradePipFirst);

  // Populate groups.
  for (int i = 0; i < static_cast<int>(spec.groups.size()); ++i)
    m_GroupsLayout->addWidget(this->CreateGroupWidget(spec.groups[i], i));

  // Populate downloads.
  for (int i = 0; i < static_cast<int>(spec.huggingFaceDownloads.size()); ++i)
    m_DownloadsLayout->addWidget(this->CreateDownloadWidget(spec.huggingFaceDownloads[i], i));
}

void QmitkPipInstallAdvancedDialog::OnAddGroup()
{
  const int index = static_cast<int>(m_GroupBoxes.size());
  m_GroupsLayout->addWidget(this->CreateGroupWidget(mitk::PipInstallGroup(), index));
}

void QmitkPipInstallAdvancedDialog::OnRemoveGroup()
{
  auto* button = qobject_cast<QPushButton*>(sender());
  if (!button)
    return;

  // Walk up from the button to find the QGroupBox it belongs to.
  auto* groupBox = qobject_cast<QGroupBox*>(button->parentWidget());
  if (!groupBox)
    return;

  auto it = std::find(m_GroupBoxes.begin(), m_GroupBoxes.end(), groupBox);
  if (it == m_GroupBoxes.end())
    return;

  int index = static_cast<int>(std::distance(m_GroupBoxes.begin(), it));

  m_GroupBoxes.erase(m_GroupBoxes.begin() + index);
  m_GroupWidgets.erase(m_GroupWidgets.begin() + index);
  delete groupBox;

  this->RenumberGroupTitles();
}

void QmitkPipInstallAdvancedDialog::OnAddDownload()
{
  const int index = static_cast<int>(m_DownloadBoxes.size());
  m_DownloadsLayout->addWidget(this->CreateDownloadWidget(mitk::HuggingFaceDownload(), index));
}

void QmitkPipInstallAdvancedDialog::OnRemoveDownload()
{
  auto* button = qobject_cast<QPushButton*>(sender());
  if (!button)
    return;

  auto* groupBox = qobject_cast<QGroupBox*>(button->parentWidget());
  if (!groupBox)
    return;

  auto it = std::find(m_DownloadBoxes.begin(), m_DownloadBoxes.end(), groupBox);
  if (it == m_DownloadBoxes.end())
    return;

  int index = static_cast<int>(std::distance(m_DownloadBoxes.begin(), it));

  m_DownloadBoxes.erase(m_DownloadBoxes.begin() + index);
  m_DownloadWidgets.erase(m_DownloadWidgets.begin() + index);
  delete groupBox;

  this->RenumberDownloadTitles();
}

void QmitkPipInstallAdvancedDialog::OnLoadSpec()
{
  auto path = QFileDialog::getOpenFileName(this, "Load Install Spec", QString(), "JSON Files (*.json)");

  if (path.isEmpty())
    return;

  try
  {
    const auto spec = mitk::PipInstallSpec::FromFile(path.toStdString());
    this->RebuildFromSpec(spec);
  }
  catch (const std::exception& e)
  {
    QMessageBox msgBox(QMessageBox::Warning, "Load failed",
      QString("The file \"%1\" could not be loaded. Make sure it is a valid MITK pip install spec (JSON).").arg(path),
      QMessageBox::Ok, this);
    msgBox.setDetailedText(QString::fromUtf8(e.what()));
    msgBox.exec();
  }
}

void QmitkPipInstallAdvancedDialog::OnSaveSpec()
{
  auto defaultName = m_NameEdit->text().trimmed();

  defaultName = !defaultName.isEmpty()
    ? defaultName.toLower().replace(' ', '_') + "_install_spec"
    : "install_spec";

  auto path = QFileDialog::getSaveFileName(this, "Save Install Spec", defaultName + ".json", "JSON Files (*.json)");

  if (path.isEmpty())
    return;

  try
  {
    const auto spec = this->GetInstallSpec();
    spec.SaveToFile(path.toStdString());
  }
  catch (const std::exception& e)
  {
    QMessageBox::warning(this, "Save failed", QString::fromUtf8(e.what()));
  }
}
