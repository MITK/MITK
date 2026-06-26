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

  // --- Post-install steps section ---

  mainLayout->addWidget(new QLabel("<b>Post-install Steps</b>"));

  m_StepsLayout = new QVBoxLayout;
  mainLayout->addLayout(m_StepsLayout);

  auto* addStepButton = new QPushButton("Add Post-install Step");
  mainLayout->addWidget(addStepButton);
  connect(addStepButton, &QPushButton::clicked, this, &QmitkPipInstallAdvancedDialog::OnAddStep);

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

  for (const auto& widgets : m_StepWidgets)
  {
    mitk::PostInstallStep step;

    step.displayName = widgets.displayName->text().trimmed().toStdString();
    step.pythonCode = widgets.pythonCode->toPlainText().toStdString();
    step.optional = widgets.optionalCheckBox->isChecked();

    spec.postInstallSteps.push_back(std::move(step));
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

QGroupBox* QmitkPipInstallAdvancedDialog::CreateStepWidget(const mitk::PostInstallStep& step, int index)
{
  auto* groupBox = new QGroupBox(QString("Post-install Step %1").arg(index + 1));
  auto* layout = new QVBoxLayout(groupBox);
  auto* formLayout = new QFormLayout;

  auto* displayName = new QLineEdit;
  displayName->setPlaceholderText("Label shown in the installer UI");
  displayName->setText(QString::fromStdString(step.displayName));
  formLayout->addRow("Display name:", displayName);

  auto* pythonCode = new QPlainTextEdit;
  pythonCode->setTabChangesFocus(true);
  pythonCode->setMinimumHeight(60);
  pythonCode->setPlaceholderText("Python source run via `python -c` in the virtual environment");
  pythonCode->setPlainText(QString::fromStdString(step.pythonCode));
  formLayout->addRow("Python code:", pythonCode);

  auto* optionalCheckBox = new QCheckBox("Optional (failure does not block installation)");
  optionalCheckBox->setChecked(step.optional);
  formLayout->addRow("", optionalCheckBox);

  layout->addLayout(formLayout);

  auto* removeButton = new QPushButton("Remove");
  layout->addWidget(removeButton, 0, Qt::AlignRight);
  connect(removeButton, &QPushButton::clicked, this, &QmitkPipInstallAdvancedDialog::OnRemoveStep);

  m_StepWidgets.push_back({ displayName, pythonCode, optionalCheckBox });
  m_StepBoxes.push_back(groupBox);

  return groupBox;
}

void QmitkPipInstallAdvancedDialog::RenumberGroupTitles()
{
  for (int i = 0; i < static_cast<int>(m_GroupBoxes.size()); ++i)
    m_GroupBoxes[i]->setTitle(QString("Install Group %1").arg(i + 1));
}

void QmitkPipInstallAdvancedDialog::RenumberStepTitles()
{
  for (int i = 0; i < static_cast<int>(m_StepBoxes.size()); ++i)
    m_StepBoxes[i]->setTitle(QString("Post-install Step %1").arg(i + 1));
}

void QmitkPipInstallAdvancedDialog::RebuildFromSpec(const mitk::PipInstallSpec& spec)
{
  // Clear existing groups.
  for (auto* box : m_GroupBoxes)
    delete box;

  m_GroupWidgets.clear();
  m_GroupBoxes.clear();

  // Clear existing post-install steps.
  for (auto* box : m_StepBoxes)
    delete box;

  m_StepWidgets.clear();
  m_StepBoxes.clear();

  // Populate general fields.
  m_NameEdit->setText(QString::fromStdString(spec.name));
  m_VenvNameEdit->setText(QString::fromStdString(spec.venvName));
  m_UpgradePipFirstCheckBox->setChecked(spec.upgradePipFirst);

  // Populate groups.
  for (int i = 0; i < static_cast<int>(spec.groups.size()); ++i)
    m_GroupsLayout->addWidget(this->CreateGroupWidget(spec.groups[i], i));

  // Populate post-install steps.
  for (int i = 0; i < static_cast<int>(spec.postInstallSteps.size()); ++i)
    m_StepsLayout->addWidget(this->CreateStepWidget(spec.postInstallSteps[i], i));
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

void QmitkPipInstallAdvancedDialog::OnAddStep()
{
  const int index = static_cast<int>(m_StepBoxes.size());
  m_StepsLayout->addWidget(this->CreateStepWidget(mitk::PostInstallStep(), index));
}

void QmitkPipInstallAdvancedDialog::OnRemoveStep()
{
  auto* button = qobject_cast<QPushButton*>(sender());
  if (!button)
    return;

  auto* groupBox = qobject_cast<QGroupBox*>(button->parentWidget());
  if (!groupBox)
    return;

  auto it = std::find(m_StepBoxes.begin(), m_StepBoxes.end(), groupBox);
  if (it == m_StepBoxes.end())
    return;

  int index = static_cast<int>(std::distance(m_StepBoxes.begin(), it));

  m_StepBoxes.erase(m_StepBoxes.begin() + index);
  m_StepWidgets.erase(m_StepWidgets.begin() + index);
  delete groupBox;

  this->RenumberStepTitles();
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
