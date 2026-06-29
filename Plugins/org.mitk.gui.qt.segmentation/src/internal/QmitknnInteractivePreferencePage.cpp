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
#include <mitknnInteractiveInstall.h>
#include <mitknnInteractiveModel.h>
#include <mitknnInteractiveUpdatePrompt.h>
#include <mitknnInteractiveVersion.h>
#include <mitkPipPackageInfo.h>
#include <mitkPythonHelper.h>
#include <QmitkPipInstallDialog.h>
#endif

#include <QmitkRun.h>

#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>

#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QLineEdit>
#include <QToolButton>

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
  QObject::connect(m_Ui->checkForUpdatesButton, &QPushButton::clicked, m_Control,
                   [this] { this->OnCheckForUpdatesButtonClicked(); });

#if !MITK_HAS_PYTHON
  m_Ui->uninstallButton->setVisible(false);
  m_Ui->checkForUpdatesButton->setVisible(false);
#endif

  connect(m_Ui->hfSourceRadioButton, &QRadioButton::toggled,
    this, &QmitknnInteractivePreferencePage::OnModelSourceToggled);
  connect(m_Ui->localSourceRadioButton, &QRadioButton::toggled,
    this, &QmitknnInteractivePreferencePage::OnModelSourceToggled);
  connect(m_Ui->localModelPathBrowseButton, &QToolButton::clicked,
    this, &QmitknnInteractivePreferencePage::OnBrowseLocalModelPath);

  // Empty means "use the recommended default"; the hint also points users at the
  // Refresh button, which lists the available checkpoints in the drop-down.
  if (auto* lineEdit = m_Ui->checkpointComboBox->lineEdit())
    lineEdit->setPlaceholderText("Default model (click Refresh to choose another)");

  connect(m_Ui->refreshModelsButton, &QToolButton::clicked,
    this, &QmitknnInteractivePreferencePage::OnRefreshModelsClicked);

#if !MITK_HAS_PYTHON
  // Model discovery needs the embedded Python environment.
  m_Ui->refreshModelsButton->setVisible(false);
#endif

  connect(m_Ui->localModeRadioButton, &QRadioButton::toggled,
    this, &QmitknnInteractivePreferencePage::OnInferenceModeToggled);
  connect(m_Ui->remoteModeRadioButton, &QRadioButton::toggled,
    this, &QmitknnInteractivePreferencePage::OnInferenceModeToggled);

  // torch.compile relies on Triton, which is unavailable outside Linux; hide the
  // option there. Its enabled state tracks the computation backend (CUDA only).
#ifndef __linux__
  m_Ui->torchCompileCheckBox->setVisible(false);
#endif
  connect(m_Ui->autoBackendRadioButton, &QRadioButton::toggled,
    this, &QmitknnInteractivePreferencePage::OnComputationBackendChanged);
  connect(m_Ui->cpuBackendRadioButton, &QRadioButton::toggled,
    this, &QmitknnInteractivePreferencePage::OnComputationBackendChanged);
  connect(m_Ui->gpuBackendRadioButton, &QRadioButton::toggled,
    this, &QmitknnInteractivePreferencePage::OnComputationBackendChanged);
  connect(m_Ui->gpuBackendLineEdit, &QLineEdit::textChanged,
    this, &QmitknnInteractivePreferencePage::OnComputationBackendChanged);

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

  prefs->PutBool("nnInteractive/useTorchCompile", m_Ui->torchCompileCheckBox->isChecked());

  if (m_Ui->blosc2StorageRadioButton->isChecked())
  {
    prefs->Put("nnInteractive/interactionsStorage", "blosc2");
  }
  else if (m_Ui->tensorStorageRadioButton->isChecked())
  {
    prefs->Put("nnInteractive/interactionsStorage", "tensor");
  }
  else
  {
    prefs->Put("nnInteractive/interactionsStorage", "auto");
  }

  prefs->Put("nnInteractive/modelCheckpoint", this->CurrentModelId().toStdString());

  prefs->Put("nnInteractive/modelSource",
    m_Ui->localSourceRadioButton->isChecked() ? "local" : "huggingface");
  prefs->Put("nnInteractive/localModelPath",
    m_Ui->localModelPathLineEdit->text().trimmed().toStdString());

  // A client-only install can only run remote sessions.
  prefs->Put("nnInteractive/inferenceMode",
    (m_ClientOnly || m_Ui->remoteModeRadioButton->isChecked()) ? "remote" : "local");
  prefs->Put("nnInteractive/serverUrl",
    m_Ui->serverUrlLineEdit->text().trimmed().toStdString());
  prefs->Put("nnInteractive/apiKey", m_Ui->apiKeyLineEdit->text().trimmed().toStdString());

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
  const auto useTorchCompile = prefs->GetBool("nnInteractive/useTorchCompile", false);
  const auto interactionsStorage = prefs->Get("nnInteractive/interactionsStorage", "auto");
  const auto modelCheckpoint = prefs->Get("nnInteractive/modelCheckpoint", "");
  const auto modelSource = prefs->Get("nnInteractive/modelSource", "huggingface");
  const auto localModelPath = prefs->Get("nnInteractive/localModelPath", "");
  const auto inferenceMode = prefs->Get("nnInteractive/inferenceMode", "local");
  const auto serverUrl = prefs->Get("nnInteractive/serverUrl", "");
  const auto apiKey = prefs->Get("nnInteractive/apiKey", "");

  m_ClientOnly = prefs->Get("nnInteractive/installMode", "full") == "client";

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

  m_Ui->torchCompileCheckBox->setChecked(useTorchCompile);

  if (interactionsStorage == "blosc2")
  {
    m_Ui->blosc2StorageRadioButton->setChecked(true);
  }
  else if (interactionsStorage == "tensor")
  {
    m_Ui->tensorStorageRadioButton->setChecked(true);
  }
  else
  {
    m_Ui->autoStorageRadioButton->setChecked(true);
  }

  // Reflect the just-restored backend selection in the torch.compile checkbox.
  this->OnComputationBackendChanged();

  // Editable combo box: without a refresh it just holds the stored id as text;
  // OnRefreshModelsClicked() populates the dropdown on demand.
  m_Ui->checkpointComboBox->setCurrentText(QString::fromStdString(modelCheckpoint));

  if (modelSource == "local")
    m_Ui->localSourceRadioButton->setChecked(true);
  else
    m_Ui->hfSourceRadioButton->setChecked(true);

  m_Ui->localModelPathLineEdit->setText(QString::fromStdString(localModelPath));

  // A client-only install supports remote inference only: force Remote and keep
  // the Local option visible but disabled, with an explanatory hint.
  if (m_ClientOnly)
  {
    m_Ui->remoteModeRadioButton->setChecked(true);
    m_Ui->localModeRadioButton->setEnabled(false);
  }
  else
  {
    m_Ui->localModeRadioButton->setEnabled(true);

    if (inferenceMode == "remote")
      m_Ui->remoteModeRadioButton->setChecked(true);
    else
      m_Ui->localModeRadioButton->setChecked(true);
  }

  m_Ui->clientOnlyHintLabel->setVisible(m_ClientOnly);

  m_Ui->serverUrlLineEdit->setText(QString::fromStdString(serverUrl));
  m_Ui->apiKeyLineEdit->setText(QString::fromStdString(apiKey));

  // Calls OnModelSourceToggled() itself when local mode is active.
  this->OnInferenceModeToggled();

  this->UpdateUninstallButton();
  this->UpdateCheckForUpdatesButton();
}

void QmitknnInteractivePreferencePage::OnModelSourceToggled()
{
  const bool local = m_Ui->localSourceRadioButton->isChecked();

  m_Ui->checkpointLabel->setEnabled(!local);
  m_Ui->checkpointComboBox->setEnabled(!local);
  m_Ui->refreshModelsButton->setEnabled(!local);

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

  // Model, compute backend, and storage selection only apply to local
  // inference; a remote server provides its own model, compute device, and
  // interaction storage.
  m_Ui->modelGroupBox->setEnabled(!remote);
  m_Ui->backendGroupBox->setEnabled(!remote);
  m_Ui->storageBackendGroupBox->setEnabled(!remote);

  if (!remote)
    this->OnModelSourceToggled();
}

void QmitknnInteractivePreferencePage::OnComputationBackendChanged()
{
  // torch.compile only makes sense on a CUDA device. The tool applies it
  // whenever the session runs on CUDA, which includes the default "auto"
  // backend when a compatible CUDA device is detected, not only the explicitly
  // forced GPU backend. So enable the option whenever the backend is not pinned
  // to CPU and the device string targets CUDA. If "auto" falls back to CPU at
  // runtime, the tool's own useCUDADevice guard neutralizes the stored
  // preference.
  const bool cudaDevice = m_Ui->gpuBackendLineEdit->text().trimmed().startsWith("cuda", Qt::CaseInsensitive);

  m_Ui->torchCompileCheckBox->setEnabled(!m_Ui->cpuBackendRadioButton->isChecked() && cudaDevice);
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

QString QmitknnInteractivePreferencePage::CurrentModelId() const
{
  // Prefer the selected item's model id (stored as userData), but only while the
  // visible text still matches that item. An editable combo keeps currentIndex
  // after the user edits the text, so itemData could otherwise return a previously
  // selected id and silently discard a typed-in value. The displayed item text is
  // a decorated label ("name (recommended)"), so matching against it here, rather
  // than against the stored id, is also what corrupts the value on a re-refresh.
  // An empty value is kept as-is and means "use the recommended default" (resolved
  // via get_default_model_id() at session start).
  const int currentIndex = m_Ui->checkpointComboBox->currentIndex();

  if (currentIndex >= 0 &&
      m_Ui->checkpointComboBox->currentText() == m_Ui->checkpointComboBox->itemText(currentIndex))
    return m_Ui->checkpointComboBox->itemData(currentIndex).toString();

  return m_Ui->checkpointComboBox->currentText().trimmed();
}

void QmitknnInteractivePreferencePage::OnRefreshModelsClicked()
{
#if MITK_HAS_PYTHON
  if (!mitk::PythonHelper::VirtualEnvExists("nnInteractive"))
  {
    QMessageBox::information(m_Control, "nnInteractive",
      "nnInteractive is not installed yet. Initialize it once, then refresh the model list.");
    return;
  }

  // Listing models refreshes the manifest from Hugging Face and spins up a
  // transient Python context. ListModels drives it behind a modal progress
  // dialog (parented to this page) so the Workbench stays responsive and the
  // user can cancel.
  auto status = mitk::nnInteractive::ModelListStatus::Failed;
  const auto models = mitk::nnInteractive::ListModels("nnInteractive", &status, m_Control);

  if (status == mitk::nnInteractive::ModelListStatus::Failed)
  {
    QMessageBox::information(m_Control, "nnInteractive",
      "Could not load the model list. Check your internet connection, or enter a model id manually.");
    return;
  }

  if (status == mitk::nnInteractive::ModelListStatus::Empty)
  {
    QMessageBox::information(m_Control, "nnInteractive",
      "nnInteractive reported no model checkpoints. Enter a model id manually.");
    return;
  }

  // Preserve the current selection by model id (not the decorated display label)
  // so a refresh never silently changes or corrupts it.
  const QString currentId = this->CurrentModelId();

  m_Ui->checkpointComboBox->clear();

  int selectIndex = -1;
  for (const auto& model : models)
  {
    const auto id = QString::fromStdString(model.Id);
    auto label = QString::fromStdString(model.DisplayName);

    // The model id is stored as the item's data; only the friendly name plus the
    // combined status tags are shown.
    QStringList tags;
    if (model.IsDefault)
      tags << "recommended";
    if (!model.Downloaded)
      tags << "requires download";
    if (!tags.isEmpty())
      label += QStringLiteral(" (%1)").arg(tags.join(", "));

    m_Ui->checkpointComboBox->addItem(label, id);

    if (id == currentId)
      selectIndex = m_Ui->checkpointComboBox->count() - 1;
  }

  if (selectIndex >= 0)
    m_Ui->checkpointComboBox->setCurrentIndex(selectIndex);
  else
    m_Ui->checkpointComboBox->setCurrentText(currentId); // Free-text id or empty default, never a decorated label.
#endif
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
    // The venv is already deleted, so installMode must reflect that immediately,
    // independent of this dialog's OK/Cancel cycle: the tool GUI mirrors it as a
    // "reflects reality" preference and observes it to refresh its Initialize
    // button. inferenceMode, in contrast, is a staged preference of this page
    // (read in Update(), written in PerformOk()), so it is not touched here; the
    // widget changes below select Local and let a subsequent OK persist it.
    GetPreferences()->Put("nnInteractive/installMode", "full");

    // No client-only restriction applies anymore: re-enable and select Local so
    // the next Initialize can install fresh and the user can choose freely.
    m_ClientOnly = false;
    m_Ui->clientOnlyHintLabel->setVisible(false);
    m_Ui->localModeRadioButton->setEnabled(true);
    m_Ui->localModeRadioButton->setChecked(true);
    this->OnInferenceModeToggled();

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

  // Both buttons act on the installed environment, so keep them in sync with it.
  this->UpdateUninstallButton();
  this->UpdateCheckForUpdatesButton();
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

void QmitknnInteractivePreferencePage::OnCheckForUpdatesButtonClicked()
{
#if MITK_HAS_PYTHON
  const bool clientOnly = GetPreferences()->Get("nnInteractive/installMode", "full") == "client";
  const std::string distributionName = clientOnly ? "nninteractive-client" : "nnInteractive";

  // The PyPI query blocks up to 5 s. Run it synchronously on the main thread
  // (the embedded Python interpreter must be driven from there) and show a wait
  // cursor so the user sees that something is happening.
  QApplication::setOverrideCursor(Qt::WaitCursor);
  const auto result = mitk::nnInteractive::CheckInstalledVersion(true, distributionName);
  QApplication::restoreOverrideCursor();

  using mitk::nnInteractive::VersionStatus;

  const bool modulesLoaded = mitk::PythonHelper::IsAnyVirtualEnvModuleLoaded("nnInteractive");

  const auto runUpdateAndReport = [&](bool client) {
    if (!this->RunUpdate(client))
      return;

    QMessageBox::information(m_Control, "nnInteractive", "nnInteractive was updated successfully.");
    this->UpdateUninstallButton();
    this->UpdateCheckForUpdatesButton();
  };

  switch (result.Status)
  {
    case VersionStatus::BelowMinimum:
    case VersionStatus::UpdateAvailable:
      // Shared version-status dialog. Not in the init flow, so declining just
      // closes; only an explicit "Update now" triggers the in-place update.
      if (mitk::nnInteractive::ShowUpdatePrompt(m_Control, result, modulesLoaded, false)
            == mitk::nnInteractive::UpdatePromptChoice::Update)
        runUpdateAndReport(clientOnly);
      break;

    case VersionStatus::UpToDate:
      // A non-empty Latest means PyPI was reached and confirmed nothing newer
      // is in range; an empty one means the query did not complete, so an
      // available update cannot be ruled out and we must not claim "up to date".
      if (!result.Latest.empty())
      {
        QMessageBox::information(
          m_Control,
          "nnInteractive",
          QString("You are up to date (v%1).")
            .arg(QString::fromStdString(result.Installed)));
      }
      else
      {
        QMessageBox::information(
          m_Control,
          "nnInteractive",
          QString(
            "<p>nnInteractive %1 is installed.</p>"
            "<p>Could not check for newer versions. Check your internet "
            "connection and try again.</p>")
            .arg(QString::fromStdString(result.Installed)));
      }
      break;

    case VersionStatus::Unknown:
      QMessageBox::warning(
        m_Control,
        "nnInteractive",
        "Could not determine the installed nnInteractive version.");
      break;
  }
#endif
}

void QmitknnInteractivePreferencePage::UpdateCheckForUpdatesButton()
{
#if MITK_HAS_PYTHON
  m_Ui->checkForUpdatesButton->setEnabled(mitk::PythonHelper::VirtualEnvExists("nnInteractive"));
#else
  m_Ui->checkForUpdatesButton->setEnabled(false);
#endif
}

bool QmitknnInteractivePreferencePage::RunUpdate(bool clientOnly)
{
#if MITK_HAS_PYTHON
  auto spec = mitk::nnInteractive::BuildUpgradeSpec("nnInteractive", clientOnly);

  QmitkPipInstallDialog dialog(spec, m_Control, QmitkPipInstallDialog::Mode::Update);

  return dialog.exec() == QDialog::Accepted;
#else
  (void)clientOnly;
  return false;
#endif
}
