/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTotalSegmentatorPreferencePage.h"
#include <ui_QmitkTotalSegmentatorPreferencePage.h>

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkSegmentationPluginConfig.h>

#include <QmitknnUNetGPU.h>
#include <QmitkRun.h>
#include <QmitkStyleManager.h>

#include <QComboBox>
#include <QFrame>
#include <QMessageBox>
#include <QPalette>
#include <QPushButton>

#if MITK_HAS_PYTHON
#include <mitkTotalSegmentatorInstall.h>
#include <mitkPythonHelper.h>

#include <QmitkVenvProcess.h>

#include <QString>
#include <QStringList>

#include <optional>
#include <utility>
#endif

namespace
{
  mitk::IPreferences *GetPreferences()
  {
    auto *preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  }

#if MITK_HAS_PYTHON
  // TotalSegmentator exposes no stable Python API for these maintenance actions
  // beyond the totalseg_set_license console script, so the rest drive its internal
  // totalsegmentator.config module through short -c snippets. Keeping the snippets
  // together documents that dependency in one place: if a future pinned release
  // renames them, this is the single spot to update.
  constexpr const char *READ_LICENSE_CODE =
    "import totalsegmentator.config as tc\n"
    "print('MITK_LICENSE:' + (tc.get_license_number() or ''))\n";

  constexpr const char *CLEAR_LICENSE_CODE =
    "import totalsegmentator.config as tc\n"
    "tc.set_license_number('', True)\n"
    "print('License removed.')\n";

  // Deletes the downloaded "nnunet" weights subtree, leaving the config file
  // (license, usage-stats opt-out) intact. rmtree runs WITHOUT ignore_errors so a
  // real failure raises and the non-zero exit is reported, instead of the process
  // always exiting 0 and the UI falsely claiming success.
  constexpr const char *CLEAR_WEIGHTS_CODE =
    "import os, shutil, totalsegmentator.config as tc\n"
    "d = os.path.join(str(tc.get_totalseg_dir()), 'nnunet')\n"
    "if os.path.isdir(d):\n"
    "    shutil.rmtree(d)\n"
    "    print('Cleared', d)\n"
    "else:\n"
    "    print('Nothing to clear at', d)\n";

  // Runs a console script / interpreter from the managed environment (with
  // PYTHONHOME/PYTHONPATH stripped by QmitkVenvProcess) and returns
  // (success, combined output).
  std::pair<bool, QString> RunVenvExecutable(const QString &executable, const QStringList &args)
  {
    const auto result = QmitkVenvProcess::Run(executable, args);
    return { result.success, result.combined() };
  }

  std::pair<bool, QString> ApplyLicense(const QString &key)
  {
    const auto executable =
      mitk::PythonHelper::GetVirtualEnvScriptPath(mitk::TotalSegmentator::VENV_NAME, "totalseg_set_license");
    if (executable.empty())
      return { false, QStringLiteral("TotalSegmentator environment not found.") };

    return RunVenvExecutable(QmitkVenvProcess::ToQString(executable), QStringList() << "-l" << key);
  }

  std::pair<bool, QString> ClearLicense()
  {
    const auto python = mitk::PythonHelper::GetVirtualEnvExecutablePath(mitk::TotalSegmentator::VENV_NAME);
    if (python.empty())
      return { false, QStringLiteral("TotalSegmentator environment not found.") };

    return RunVenvExecutable(QmitkVenvProcess::ToQString(python), QStringList() << "-c" << CLEAR_LICENSE_CODE);
  }

  // Reads the license number stored in TotalSegmentator's config. Returns the
  // license (possibly empty = none set) when it could be read, or std::nullopt
  // when the query failed (so the caller leaves the cached state untouched). Only
  // stdout is parsed and the value is sentinel-wrapped, so an import banner on
  // stderr cannot be mistaken for a license. Blocks, so call it from a worker
  // thread (see DetectLicense), never directly on the GUI thread.
  std::optional<QString> ReadLicense()
  {
    const auto python = mitk::PythonHelper::GetVirtualEnvExecutablePath(mitk::TotalSegmentator::VENV_NAME);
    if (python.empty())
      return std::nullopt;

    const auto result =
      QmitkVenvProcess::Run(QmitkVenvProcess::ToQString(python), QStringList() << "-c" << READ_LICENSE_CODE, 30000);

    if (!result.success)
      return std::nullopt;

    const QString marker = QStringLiteral("MITK_LICENSE:");
    for (const auto &line : result.standardOutput.split('\n'))
    {
      if (line.startsWith(marker))
        return line.mid(marker.size()).trimmed();
    }

    return std::nullopt;
  }

  std::pair<bool, QString> ClearModelWeights()
  {
    const auto python = mitk::PythonHelper::GetVirtualEnvExecutablePath(mitk::TotalSegmentator::VENV_NAME);
    if (python.empty())
      return { false, QStringLiteral("TotalSegmentator environment not found.") };

    return RunVenvExecutable(QmitkVenvProcess::ToQString(python), QStringList() << "-c" << CLEAR_WEIGHTS_CODE);
  }
#endif
}

QmitkTotalSegmentatorPreferencePage::QmitkTotalSegmentatorPreferencePage()
  : m_Ui(std::make_unique<Ui::QmitkTotalSegmentatorPreferencePage>()), m_Control(nullptr)
{
}

QmitkTotalSegmentatorPreferencePage::~QmitkTotalSegmentatorPreferencePage()
{
}

void QmitkTotalSegmentatorPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkTotalSegmentatorPreferencePage::CreateQtControl(QWidget *parent)
{
  m_Control = new QWidget(parent);
  m_Ui->setupUi(m_Control);

  QmitkGPULoader gpuLoader;
  m_Ui->deviceComboBox->addItem("Automatic GPU", "gpu");
  for (const auto &spec : gpuLoader.GetAllGPUSpecs())
  {
    m_Ui->deviceComboBox->addItem(
      QString("GPU %1: %2 (%3)").arg(spec.id).arg(spec.name, spec.memory), QString("gpu:%1").arg(spec.id));
  }
  m_Ui->deviceComboBox->addItem("CPU", "cpu");

  // MITK has no themed-link helper, and the default anchor colour is a dark blue
  // that is hard to read on the dark theme. Colour the links from the active
  // theme via QmitkStyleManager (its accent colour is parsed from the current
  // light/dark stylesheet) and present the text as a bordered, padded info panel,
  // so it reads well in both themes without hardcoding a colour.
  const QString linkColor = QmitkStyleManager::GetIconAccentColor();
  m_Ui->licenseInfoLabel->setTextFormat(Qt::RichText);
  m_Ui->licenseInfoLabel->setOpenExternalLinks(true);
  m_Ui->licenseInfoLabel->setWordWrap(true);
  m_Ui->licenseInfoLabel->setFrameShape(QFrame::StyledPanel);
  m_Ui->licenseInfoLabel->setMargin(6);
  m_Ui->licenseInfoLabel->setAutoFillBackground(true);
  m_Ui->licenseInfoLabel->setBackgroundRole(QPalette::Base);
  m_Ui->licenseInfoLabel->setText(
    QString("Some tasks require a TotalSegmentator license. Request an academic (non-commercial) license at "
            "<a href=\"https://backend.totalsegmentator.com/license-academic/\">"
            "<span style=\"text-decoration: underline; color:%1;\">backend.totalsegmentator.com</span></a>; "
            "for commercial use, contact "
            "<a href=\"mailto:jakob.wasserthal@usb.ch\">"
            "<span style=\"text-decoration: underline; color:%1;\">jakob.wasserthal@usb.ch</span></a>.")
      .arg(linkColor));

  connect(m_Ui->applyLicenseButton, &QPushButton::clicked, m_Control, [this] { this->OnApplyLicenseClicked(); });
  connect(m_Ui->removeLicenseButton, &QPushButton::clicked, m_Control, [this] { this->OnRemoveLicenseClicked(); });
  connect(m_Ui->clearWeightsButton, &QPushButton::clicked, m_Control, [this] { this->OnClearModelWeightsClicked(); });
  connect(m_Ui->uninstallButton, &QPushButton::clicked, m_Control, [this] { this->OnUninstallButtonClicked(); });

  this->Update();
}

QWidget *QmitkTotalSegmentatorPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkTotalSegmentatorPreferencePage::PerformOk()
{
  GetPreferences()->Put("TotalSeg/device", m_Ui->deviceComboBox->currentData().toString().toStdString());
  return true;
}

void QmitkTotalSegmentatorPreferencePage::PerformCancel()
{
  // Only the compute device is part of the OK/Cancel transaction (it is committed
  // in PerformOk). The license and maintenance actions run immediately against the
  // environment when their buttons are pressed and cannot be rolled back, so there
  // is deliberately nothing to revert here.
}

void QmitkTotalSegmentatorPreferencePage::Update()
{
  const QString device = QString::fromStdString(GetPreferences()->Get("TotalSeg/device", "gpu"));
  int index = m_Ui->deviceComboBox->findData(device);
  if (index < 0 && !device.isEmpty())
  {
    m_Ui->deviceComboBox->addItem(device, device);
    index = m_Ui->deviceComboBox->count() - 1;
  }
  if (index >= 0)
    m_Ui->deviceComboBox->setCurrentIndex(index);

#if MITK_HAS_PYTHON
  if (mitk::PythonHelper::VirtualEnvExists(mitk::TotalSegmentator::VENV_NAME))
    this->DetectLicense();
#endif

  this->RefreshState();
}

void QmitkTotalSegmentatorPreferencePage::DetectLicense()
{
#if MITK_HAS_PYTHON
  // Read the license on a worker thread: it cold-starts the venv interpreter and
  // imports totalsegmentator (which pulls in torch), so running it directly on the
  // GUI thread froze the preferences dialog for seconds on every open.
  const auto license = QmitkRunAsyncBlocking<std::optional<QString>>(
    "TotalSegmentator", "Reading the TotalSegmentator license...", [] { return ReadLicense(); });

  // Leave the cached state untouched if the license could not be determined
  // (e.g. a transient subprocess failure), rather than falsely clearing it. This
  // is the single place that mirrors the authoritative venv config into the cached
  // TotalSeg/hasLicense preference the tool GUI reads.
  if (!license.has_value())
    return;

  m_Ui->licenseLineEdit->setText(*license);
  GetPreferences()->PutBool("TotalSeg/hasLicense", !license->isEmpty());
#endif
}

void QmitkTotalSegmentatorPreferencePage::RefreshState()
{
#if MITK_HAS_PYTHON
  const bool installed = mitk::PythonHelper::VirtualEnvExists(mitk::TotalSegmentator::VENV_NAME);
  const bool hasLicense = GetPreferences()->GetBool("TotalSeg/hasLicense", false);

  // The info label (with its "how to obtain a license" links) stays live
  // regardless of install state; only the actions that need the venv are gated.
  m_Ui->licenseLineEdit->setEnabled(installed);
  m_Ui->applyLicenseButton->setEnabled(installed);
  m_Ui->removeLicenseButton->setEnabled(installed);
  m_Ui->clearWeightsButton->setEnabled(installed);
  m_Ui->uninstallButton->setEnabled(installed);

  if (installed)
  {
    m_Ui->statusLabel->setText(hasLicense ? "TotalSegmentator is installed. A license is set."
                                          : "TotalSegmentator is installed.");
  }
  else
  {
    m_Ui->statusLabel->setText(
      "TotalSegmentator is not installed. Install it from the TotalSegmentator tool in the Segmentation view.");
  }
#else
  m_Ui->licenseGroupBox->setVisible(false);
  m_Ui->maintenanceGroupBox->setVisible(false);
  m_Ui->statusLabel->setText("TotalSegmentator requires a Python-enabled build of MITK.");
#endif
}

void QmitkTotalSegmentatorPreferencePage::OnApplyLicenseClicked()
{
#if MITK_HAS_PYTHON
  const QString key = m_Ui->licenseLineEdit->text().trimmed();

  if (key.isEmpty())
  {
    QMessageBox::information(m_Control, "TotalSegmentator", "Enter a license key first, or use Remove to clear it.");
    return;
  }

  if (key.length() != 18)
  {
    QMessageBox::warning(m_Control, "TotalSegmentator", "Invalid license key. 18 characters expected.");
    return;
  }

  const auto result = QmitkRunAsyncBlocking<std::pair<bool, QString>>(
    "TotalSegmentator", "Applying license...", [key] { return ApplyLicense(key); });

  GetPreferences()->PutBool("TotalSeg/hasLicense", result.first);

  if (result.first)
    QMessageBox::information(m_Control, "TotalSegmentator", "License applied.");
  else
    QMessageBox::critical(m_Control, "TotalSegmentator",
                          result.second.isEmpty() ? QStringLiteral("Failed to apply the license.") : result.second);

  this->RefreshState();
#endif
}

void QmitkTotalSegmentatorPreferencePage::OnRemoveLicenseClicked()
{
#if MITK_HAS_PYTHON
  const auto result = QmitkRunAsyncBlocking<std::pair<bool, QString>>(
    "TotalSegmentator", "Removing license...", [] { return ClearLicense(); });

  GetPreferences()->PutBool("TotalSeg/hasLicense", false);
  m_Ui->licenseLineEdit->clear();

  if (result.first)
    QMessageBox::information(m_Control, "TotalSegmentator", "License removed.");
  else
    QMessageBox::critical(m_Control, "TotalSegmentator",
                          result.second.isEmpty() ? QStringLiteral("Failed to remove the license.") : result.second);

  this->RefreshState();
#endif
}

void QmitkTotalSegmentatorPreferencePage::OnClearModelWeightsClicked()
{
#if MITK_HAS_PYTHON
  const auto answer = QMessageBox::question(
    m_Control, "TotalSegmentator",
    "Delete all downloaded TotalSegmentator model weights? They are re-downloaded automatically "
    "when a task next needs them.",
    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (answer != QMessageBox::Yes)
    return;

  const auto result = QmitkRunAsyncBlocking<std::pair<bool, QString>>(
    "TotalSegmentator", "Clearing model weights...", [] { return ClearModelWeights(); });

  if (result.first)
    QMessageBox::information(m_Control, "TotalSegmentator", "Model weights cleared.");
  else
    QMessageBox::critical(m_Control, "TotalSegmentator",
                          result.second.isEmpty() ? QStringLiteral("Failed to clear model weights.") : result.second);
#endif
}

void QmitkTotalSegmentatorPreferencePage::OnUninstallButtonClicked()
{
#if MITK_HAS_PYTHON
  const auto answer = QMessageBox::warning(
    m_Control, "Uninstall TotalSegmentator",
    "Are you sure you want to remove the TotalSegmentator virtual environment?", QMessageBox::Yes | QMessageBox::No,
    QMessageBox::No);

  if (answer != QMessageBox::Yes)
    return;

  const bool removed = QmitkRunAsyncBlocking<bool>(
    "Uninstall TotalSegmentator", "Removing the TotalSegmentator virtual environment...",
    [] { return mitk::PythonHelper::RemoveVirtualEnv(mitk::TotalSegmentator::VENV_NAME); });

  if (removed)
  {
    GetPreferences()->PutBool("TotalSeg/hasLicense", false);
    QMessageBox::information(m_Control, "TotalSegmentator", "TotalSegmentator was uninstalled successfully.");
  }
  else
  {
    QMessageBox::critical(m_Control, "TotalSegmentator",
                          "Failed to remove the TotalSegmentator virtual environment. "
                          "Make sure no process is using it and try again.");
  }

  this->RefreshState();
#endif
}
