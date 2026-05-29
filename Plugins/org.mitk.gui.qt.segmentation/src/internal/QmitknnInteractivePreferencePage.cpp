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

#include <QCoreApplication>
#include <QMessageBox>

#include <QDir>
#include <QFileDialog>

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
  m_Ui->uninstallButton->setVisible(false);
#endif

  connect(m_Ui->hfSourceRadioButton, &QRadioButton::toggled,
    this, &QmitknnInteractivePreferencePage::OnModelSourceToggled);
  connect(m_Ui->localSourceRadioButton, &QRadioButton::toggled,
    this, &QmitknnInteractivePreferencePage::OnModelSourceToggled);
  connect(m_Ui->localModelPathBrowseButton, &QToolButton::clicked,
    this, &QmitknnInteractivePreferencePage::OnBrowseLocalModelPath);

  connect(m_Ui->localModeRadioButton, &QRadioButton::toggled,
    this, &QmitknnInteractivePreferencePage::OnInferenceModeToggled);
  connect(m_Ui->remoteModeRadioButton, &QRadioButton::toggled,
    this, &QmitknnInteractivePreferencePage::OnInferenceModeToggled);

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

  prefs->Put("nnInteractive/modelSource",
    m_Ui->localSourceRadioButton->isChecked() ? "local" : "huggingface");
  prefs->Put("nnInteractive/localModelPath",
    m_Ui->localModelPathLineEdit->text().trimmed().toStdString());

  prefs->Put("nnInteractive/inferenceMode",
    m_Ui->remoteModeRadioButton->isChecked() ? "remote" : "local");
  prefs->Put("nnInteractive/serverUrl",
    m_Ui->serverUrlLineEdit->text().trimmed().toStdString());
  prefs->Put("nnInteractive/apiKey", m_Ui->apiKeyLineEdit->text().toStdString());

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
  const auto modelSource = prefs->Get("nnInteractive/modelSource", "huggingface");
  const auto localModelPath = prefs->Get("nnInteractive/localModelPath", "");
  const auto inferenceMode = prefs->Get("nnInteractive/inferenceMode", "local");
  const auto serverUrl = prefs->Get("nnInteractive/serverUrl", "");
  const auto apiKey = prefs->Get("nnInteractive/apiKey", "");

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

  if (modelSource == "local")
    m_Ui->localSourceRadioButton->setChecked(true);
  else
    m_Ui->hfSourceRadioButton->setChecked(true);

  m_Ui->localModelPathLineEdit->setText(QString::fromStdString(localModelPath));

  if (inferenceMode == "remote")
    m_Ui->remoteModeRadioButton->setChecked(true);
  else
    m_Ui->localModeRadioButton->setChecked(true);

  m_Ui->serverUrlLineEdit->setText(QString::fromStdString(serverUrl));
  m_Ui->apiKeyLineEdit->setText(QString::fromStdString(apiKey));

  // Calls OnModelSourceToggled() itself when local mode is active.
  this->OnInferenceModeToggled();

  this->UpdateUninstallButton();
}

void QmitknnInteractivePreferencePage::OnModelSourceToggled()
{
  const bool local = m_Ui->localSourceRadioButton->isChecked();

  m_Ui->checkpointLabel->setEnabled(!local);
  m_Ui->checkpointLineEdit->setEnabled(!local);

  m_Ui->localModelPathLabel->setEnabled(local);
  m_Ui->localModelPathLineEdit->setEnabled(local);
  m_Ui->localModelPathBrowseButton->setEnabled(local);
}

void QmitknnInteractivePreferencePage::OnInferenceModeToggled()
{
  const bool remote = m_Ui->remoteModeRadioButton->isChecked();

  // The server connection controls only apply to remote inference.
  m_Ui->serverUrlLabel->setEnabled(remote);
  m_Ui->serverUrlLineEdit->setEnabled(remote);
  m_Ui->apiKeyLabel->setEnabled(remote);
  m_Ui->apiKeyLineEdit->setEnabled(remote);
  m_Ui->remoteHelpLabel->setEnabled(remote);

  // Model and backend selection only apply to local inference; a remote server
  // provides its own model and compute device.
  m_Ui->modelGroupBox->setEnabled(!remote);
  m_Ui->backendGroupBox->setEnabled(!remote);

  if (!remote)
    this->OnModelSourceToggled();
}

void QmitknnInteractivePreferencePage::OnBrowseLocalModelPath()
{
  auto seed = m_Ui->localModelPathLineEdit->text();
  if (seed.isEmpty())
    seed = QDir::homePath();

  const auto selected = QFileDialog::getExistingDirectory(
    m_Control,
    QStringLiteral("Select nnInteractive checkpoint folder"),
    seed);

  if (!selected.isEmpty())
    m_Ui->localModelPathLineEdit->setText(QDir::toNativeSeparators(selected));
}

void QmitknnInteractivePreferencePage::OnUninstallButtonClicked()
{
#if MITK_HAS_PYTHON
  if (mitk::PythonHelper::IsAnyVirtualEnvModuleLoaded("nnInteractive"))
  {
    const auto appName = QCoreApplication::applicationName();
    const auto restartTarget = appName.isEmpty()
      ? QStringLiteral("this application")
      : appName;

    QMessageBox::information(
      m_Control,
      "Uninstall nnInteractive",
      QStringLiteral(
        "<p>nnInteractive cannot be uninstalled right now because Python "
        "modules from its virtual environment are still loaded.</p>"
        "<p>Restart %1 and try again.</p>").arg(restartTarget));
    return;
  }

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
