/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkVoxTellPreferencePage.h"
#include <ui_QmitkVoxTellPreferencePage.h>

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkSegmentationPluginConfig.h>

#include <QmitknnUNetGPU.h>
#include <QmitkRun.h>

#include <QComboBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>

#if MITK_HAS_PYTHON
#include <mitkPythonHelper.h>
#include <mitkPythonPackageUpdatePrompt.h>
#include <mitkVoxTellInstall.h>
#include <mitkVoxTellTool.h>

#include <QmitkPipInstallDialog.h>

#include <QApplication>
#include <QCoreApplication>
#endif

namespace
{
  mitk::IPreferences *GetPreferences()
  {
    auto *preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  }
}

QmitkVoxTellPreferencePage::QmitkVoxTellPreferencePage()
  : m_Ui(std::make_unique<Ui::QmitkVoxTellPreferencePage>()), m_Control(nullptr)
{
}

QmitkVoxTellPreferencePage::~QmitkVoxTellPreferencePage()
{
}

void QmitkVoxTellPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkVoxTellPreferencePage::CreateQtControl(QWidget *parent)
{
  m_Control = new QWidget(parent);
  m_Ui->setupUi(m_Control);

  // The data of an entry is what is stored: "auto", "cpu", or the torch device
  // of a GPU, which the preferences enforce.
  QmitkGPULoader gpuLoader;
  m_Ui->deviceComboBox->addItem("Automatic", "auto");
  for (const auto &spec : gpuLoader.GetAllGPUSpecs())
  {
    m_Ui->deviceComboBox->addItem(
      QString("GPU %1: %2 (%3)").arg(spec.id).arg(spec.name, spec.memory), QString("cuda:%1").arg(spec.id));
  }
  m_Ui->deviceComboBox->addItem("CPU", "cpu");

  connect(m_Ui->localRadioButton, &QRadioButton::toggled, m_Control, [this] { this->OnModelSourceChanged(); });
  connect(m_Ui->browseButton, &QPushButton::clicked, m_Control, [this] { this->OnBrowseButtonClicked(); });
  connect(m_Ui->checkForUpdatesButton, &QPushButton::clicked, m_Control, [this] { this->OnCheckForUpdatesButtonClicked(); });
  connect(m_Ui->uninstallButton, &QPushButton::clicked, m_Control, [this] { this->OnUninstallButtonClicked(); });

  this->Update();
}

QWidget *QmitkVoxTellPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkVoxTellPreferencePage::PerformOk()
{
  auto *prefs = GetPreferences();

  const auto device = m_Ui->deviceComboBox->currentData().toString();

  if (device == "auto")
  {
    prefs->Put("VoxTell/backend", "auto");
    prefs->Put("VoxTell/gpuBackend", "cuda:0");
  }
  else if (device == "cpu")
  {
    prefs->Put("VoxTell/backend", "cpu");
  }
  else
  {
    // Somebody who picks a GPU from the list wants that one, even if it is below
    // what is assumed to be needed.
    prefs->Put("VoxTell/backend", "gpu");
    prefs->Put("VoxTell/gpuBackend", device.toStdString());
  }

  prefs->Put("VoxTell/modelSource", m_Ui->localRadioButton->isChecked() ? "local" : "huggingface");
  prefs->Put("VoxTell/localModelPath", m_Ui->localPathLineEdit->text().trimmed().toStdString());

  return true;
}

void QmitkVoxTellPreferencePage::PerformCancel()
{
  // Everything but the uninstall is part of the OK/Cancel transaction (it is
  // committed in PerformOk). Uninstalling is done when its button is pressed
  // and cannot be rolled back, so there is deliberately nothing to revert here.
}

void QmitkVoxTellPreferencePage::Update()
{
  auto *prefs = GetPreferences();

  const auto backend = prefs->Get("VoxTell/backend", "auto");
  const auto gpuBackend = QString::fromStdString(prefs->Get("VoxTell/gpuBackend", "cuda:0"));

  const QString device = backend == "cpu" ? QStringLiteral("cpu") : (backend == "gpu" ? gpuBackend : QStringLiteral("auto"));

  // A GPU that is not listed any more (or that nvidia-smi does not report) stays
  // selectable, so opening the page and pressing OK does not change it.
  int index = m_Ui->deviceComboBox->findData(device);
  if (index < 0)
  {
    m_Ui->deviceComboBox->addItem(device, device);
    index = m_Ui->deviceComboBox->count() - 1;
  }
  m_Ui->deviceComboBox->setCurrentIndex(index);

  const bool useLocalModel = prefs->Get("VoxTell/modelSource", "huggingface") == "local";
  m_Ui->localRadioButton->setChecked(useLocalModel);
  m_Ui->huggingFaceRadioButton->setChecked(!useLocalModel);
  m_Ui->localPathLineEdit->setText(QString::fromStdString(prefs->Get("VoxTell/localModelPath", "")));

  this->OnModelSourceChanged();
  this->RefreshState();
}

void QmitkVoxTellPreferencePage::OnModelSourceChanged()
{
  const bool useLocalModel = m_Ui->localRadioButton->isChecked();

  m_Ui->localPathLineEdit->setEnabled(useLocalModel);
  m_Ui->browseButton->setEnabled(useLocalModel);
}

void QmitkVoxTellPreferencePage::OnBrowseButtonClicked()
{
  const auto folder = QFileDialog::getExistingDirectory(
    m_Control, "Select the folder of the VoxTell model", m_Ui->localPathLineEdit->text());

  if (!folder.isEmpty())
    m_Ui->localPathLineEdit->setText(folder);
}

void QmitkVoxTellPreferencePage::RefreshState()
{
#if MITK_HAS_PYTHON
  const bool installed = mitk::PythonHelper::VirtualEnvExists(mitk::VoxTell::VENV_NAME);

  m_Ui->checkForUpdatesButton->setEnabled(installed);
  m_Ui->uninstallButton->setEnabled(installed);

  m_Ui->statusLabel->setText(
    installed ? "VoxTell is installed."
              : "VoxTell is not installed. Install it from the VoxTell tool in the Segmentation view.");
#else
  m_Ui->computationGroupBox->setVisible(false);
  m_Ui->modelGroupBox->setVisible(false);
  m_Ui->maintenanceGroupBox->setVisible(false);
  m_Ui->statusLabel->setText("VoxTell requires a Python-enabled build of MITK.");
#endif
}

void QmitkVoxTellPreferencePage::OnCheckForUpdatesButtonClicked()
{
#if MITK_HAS_PYTHON
  // The PyPI query blocks up to 5 s. Run it synchronously on the main thread
  // (the embedded Python interpreter must be driven from there) and show a wait
  // cursor so the user sees that something is happening.
  QApplication::setOverrideCursor(Qt::WaitCursor);
  const auto result = mitk::PythonPackage::CheckInstalledVersion(
    mitk::VoxTell::VENV_NAME, mitk::VoxTell::DISTRIBUTION_NAME, mitk::VoxTell::SupportedVersions());
  QApplication::restoreOverrideCursor();

  using mitk::PythonPackage::VersionStatus;

  switch (result.Status)
  {
    case VersionStatus::BelowMinimum:
    case VersionStatus::UpdateAvailable:
    {
      const bool modulesLoaded = mitk::PythonHelper::IsAnyVirtualEnvModuleLoaded(mitk::VoxTell::VENV_NAME);

      // Not in the init flow, so declining just closes; only an explicit
      // "Update now" triggers the in-place update.
      const auto choice = mitk::PythonPackage::ShowUpdatePrompt(
        m_Control, "VoxTell", mitk::VoxTell::SupportedVersions(), result, modulesLoaded, false);

      if (choice != mitk::PythonPackage::UpdatePromptChoice::Update)
        break;

      QmitkPipInstallDialog dialog(mitk::VoxTell::BuildUpgradeSpec(mitk::VoxTell::VENV_NAME), m_Control, QmitkPipInstallDialog::Mode::Update);

      if (dialog.exec() == QDialog::Accepted)
        QMessageBox::information(m_Control, "VoxTell", "VoxTell was updated successfully.");

      break;
    }

    case VersionStatus::UpToDate:
      // A non-empty Latest means PyPI was reached and confirmed nothing newer
      // is in range; an empty one means the query did not complete, so an
      // available update cannot be ruled out.
      if (!result.Latest.empty())
      {
        QMessageBox::information(m_Control, "VoxTell",
          QString("You are up to date (VoxTell %1).").arg(QString::fromStdString(result.Installed)));
      }
      else
      {
        QMessageBox::information(m_Control, "VoxTell",
          QString("<p>VoxTell %1 is installed.</p>"
                  "<p>Could not check for newer versions. Check your internet connection and try again.</p>")
            .arg(QString::fromStdString(result.Installed)));
      }
      break;

    case VersionStatus::Unknown:
      QMessageBox::warning(m_Control, "VoxTell", "Could not determine the installed VoxTell version.");
      break;
  }

  this->RefreshState();
#endif
}

void QmitkVoxTellPreferencePage::OnUninstallButtonClicked()
{
#if MITK_HAS_PYTHON
  // The model runs in this process, which maps native libraries of the virtual
  // environment. They are not released until the application ends, and on
  // Windows pip and the file system cannot remove what is mapped.
  if (mitk::PythonHelper::IsAnyVirtualEnvModuleLoaded(mitk::VoxTell::VENV_NAME))
  {
    const auto appName = QCoreApplication::applicationName();
    const auto restartTarget = appName.isEmpty() ? QStringLiteral("this application") : appName;

    QMessageBox::information(
      m_Control,
      "Uninstall VoxTell",
      QStringLiteral(
        "<p>VoxTell cannot be uninstalled right now because Python modules from its "
        "virtual environment are still loaded.</p>"
        "<p>Restart %1 and try again.</p>").arg(restartTarget));
    return;
  }

  const auto answer = QMessageBox::warning(
    m_Control, "Uninstall VoxTell",
    "Are you sure you want to remove the VoxTell virtual environment? The downloaded model stays in the "
    "cache of Hugging Face and is used again if VoxTell is installed later.",
    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (answer != QMessageBox::Yes)
    return;

  const bool removed = QmitkRunAsyncBlocking<bool>(
    "Uninstall VoxTell", "Removing the VoxTell virtual environment...",
    [] { return mitk::PythonHelper::RemoveVirtualEnv(mitk::VoxTell::VENV_NAME); });

  if (removed)
  {
    QMessageBox::information(m_Control, "VoxTell", "VoxTell was uninstalled successfully.");
  }
  else
  {
    QMessageBox::critical(m_Control, "VoxTell",
                          "Failed to remove the VoxTell virtual environment. "
                          "Make sure no process is using it and try again.");
  }

  this->RefreshState();
#endif
}
