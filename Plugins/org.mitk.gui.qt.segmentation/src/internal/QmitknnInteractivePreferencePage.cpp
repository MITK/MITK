/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitknnInteractivePreferencePage.h"
#include <ui_QmitknnInteractivePreferencePage.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>
#include <mitkSegmentationPluginConfig.h>

#if MITK_HAS_PYTHON
#include <mitkPythonHelper.h>
#endif

#include <QmitkRun.h>

#include <QMessageBox>

namespace
{
  mitk::IPreferences* GetPreferences()
  {
    auto prefService = mitk::CoreServices::GetPreferencesService();
    return prefService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  }
}

QmitknnInteractivePreferencePage::QmitknnInteractivePreferencePage()
  : m_Ui(std::make_unique<Ui::QmitknnInteractivePreferencePage>()),
    m_Control(nullptr)
{
}

QmitknnInteractivePreferencePage::~QmitknnInteractivePreferencePage()
{
}

void QmitknnInteractivePreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitknnInteractivePreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);
  m_Ui->setupUi(m_Control);

  auto syncSkipNamingPromptEnabled = [this](bool enabled) {
    m_Ui->skipNamingPromptCheckBox->setEnabled(enabled);
    m_Ui->skipNamingPromptDescriptionLabel->setEnabled(enabled);
  };
  QObject::connect(m_Ui->autoCreateNextLabelCheckBox, &QCheckBox::toggled, m_Control, syncSkipNamingPromptEnabled);

  QObject::connect(m_Ui->uninstallButton, &QPushButton::clicked, m_Control,
                   [this] { this->OnUninstallButtonClicked(); });

#if !MITK_HAS_PYTHON
  m_Ui->installationGroupBox->setVisible(false);
#endif

  this->Update();
}

QWidget* QmitknnInteractivePreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitknnInteractivePreferencePage::PerformOk()
{
  auto prefs = GetPreferences();

  prefs->PutBool("nnInteractive/autoCreateNextLabel", m_Ui->autoCreateNextLabelCheckBox->isChecked());
  prefs->PutBool("nnInteractive/autoCreateNextLabelSkipNamingPrompt", m_Ui->skipNamingPromptCheckBox->isChecked());
  prefs->PutBool("nnInteractive/autoConfirm", m_Ui->autoConfirmCheckBox->isChecked());
  prefs->PutBool("nnInteractive/showShortcutsInLabels", m_Ui->showShortcutsInLabelsCheckBox->isChecked());

  if (m_Ui->cpuBackendRadioButton->isChecked())
  {
    prefs->Put("nnInteractive/backend", "cpu");
  }
  else if (m_Ui->gpuBackendRadioButton->isChecked())
  {
    prefs->Put("nnInteractive/backend", "gpu");
  }
  else
  {
    prefs->Put("nnInteractive/backend", "auto");
  }

  auto gpuBackend = m_Ui->gpuBackendLineEdit->text().toStdString();

  if (gpuBackend.empty())
    gpuBackend = "cuda:0";

  prefs->Put("nnInteractive/gpuBackend", gpuBackend);

  auto modelCheckpoint = m_Ui->checkpointLineEdit->text().toStdString();

  if (modelCheckpoint.empty())
    modelCheckpoint = "nnInteractive_v1.0";

  prefs->Put("nnInteractive/modelCheckpoint", modelCheckpoint);

  return true;
}

void QmitknnInteractivePreferencePage::PerformCancel()
{
}

void QmitknnInteractivePreferencePage::Update()
{
  const auto prefs = GetPreferences();
  const auto autoCreateNextLabel = prefs->GetBool("nnInteractive/autoCreateNextLabel", true);
  const auto skipNamingPrompt = prefs->GetBool("nnInteractive/autoCreateNextLabelSkipNamingPrompt", true);
  const auto autoConfirm = prefs->GetBool("nnInteractive/autoConfirm", false);
  const auto showShortcutsInLabels = prefs->GetBool("nnInteractive/showShortcutsInLabels", true);
  const auto backend = prefs->Get("nnInteractive/backend", "auto");
  const auto gpuBackend = prefs->Get("nnInteractive/gpuBackend", "cuda:0");
  const auto modelCheckpoint = prefs->Get("nnInteractive/modelCheckpoint", "nnInteractive_v1.0");

  m_Ui->autoCreateNextLabelCheckBox->setChecked(autoCreateNextLabel);
  m_Ui->skipNamingPromptCheckBox->setChecked(skipNamingPrompt);
  m_Ui->skipNamingPromptCheckBox->setEnabled(autoCreateNextLabel);
  m_Ui->skipNamingPromptDescriptionLabel->setEnabled(autoCreateNextLabel);
  m_Ui->autoConfirmCheckBox->setChecked(autoConfirm);
  m_Ui->showShortcutsInLabelsCheckBox->setChecked(showShortcutsInLabels);

  if (backend == "cpu")
  {
    m_Ui->cpuBackendRadioButton->setChecked(true);
  }
  else if (backend == "gpu")
  {
    m_Ui->gpuBackendRadioButton->setChecked(true);
  }
  else
  {
    m_Ui->autoBackendRadioButton->setChecked(true);
  }
  
  m_Ui->gpuBackendLineEdit->setText(QString::fromStdString(gpuBackend));

  m_Ui->checkpointLineEdit->setText(QString::fromStdString(modelCheckpoint));

  this->UpdateUninstallButton();
}

void QmitknnInteractivePreferencePage::OnUninstallButtonClicked()
{
#if MITK_HAS_PYTHON
  const auto answer = QMessageBox::warning(
    m_Control,
    "Uninstall nnInteractive",
    "<p>Are you sure you want to uninstall nnInteractive?</p>"
    "<p><b>Important:</b> This can only be safely done if nnInteractive has not "
    "been initialized since application start. If it has, restart "
    "the application before uninstalling.</p>",
    QMessageBox::Yes | QMessageBox::No,
    QMessageBox::No);

  if (answer != QMessageBox::Yes)
    return;

  const bool removed = QmitkRunAsyncBlocking<bool>(
    "Uninstall nnInteractive",
    "Removing the nnInteractive virtual environment...",
    [] { return mitk::PythonHelper::RemoveVirtualEnv("nnInteractive"); });

  if (removed)
  {
    QMessageBox::information(
      m_Control,
      "Uninstall nnInteractive",
      "nnInteractive was uninstalled successfully.");
  }
  else
  {
    QMessageBox::critical(
      m_Control,
      "Uninstall nnInteractive",
      "Failed to remove the nnInteractive virtual environment. "
      "Make sure no process is using it and try again.");
  }

  this->UpdateUninstallButton();
#endif
}

void QmitknnInteractivePreferencePage::UpdateUninstallButton()
{
#if MITK_HAS_PYTHON
  m_Ui->uninstallButton->setEnabled(mitk::PythonHelper::VirtualEnvExists("nnInteractive"));
#else
  m_Ui->uninstallButton->setEnabled(false);
#endif
}
