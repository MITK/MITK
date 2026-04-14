/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkPipInstallDialog.h>
#include <ui_QmitkPipInstallDialog.h>

#include <mitkPipInstaller.h>
#include <mitkPythonHelper.h>

#include <QCloseEvent>
#include <QMessageBox>
#include <QPushButton>

QmitkPipInstallDialog::QmitkPipInstallDialog(const mitk::PipInstallSpec& spec, QWidget* parent)
  : QDialog(parent),
    m_Ui(std::make_unique<Ui::QmitkPipInstallDialog>()),
    m_Installer(new mitk::PipInstaller(this)),
    m_Spec(spec)
{
  m_Ui->setupUi(this);

  auto name = QString::fromStdString(spec.name);

  this->setWindowTitle(name + " Installer");

  m_Ui->descriptionLabel->setText(
    QString("<h3>Do you want to install %1?</h3>"
            "<p>This may take a while depending on your internet connection.</p>")
      .arg(name.toHtmlEscaped()));

  // Hide progress elements until installation starts.
  m_Ui->statusLabel->hide();
  m_Ui->progressBar->hide();
  m_Ui->packageLabel->hide();

  // Rename the Yes button to "Install".
  if (auto* button = m_Ui->buttonBox->button(QDialogButtonBox::Yes))
    button->setText("Install " + name);

  // Wire the Install (Yes) button to our slot instead of the default accept.
  disconnect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QmitkPipInstallDialog::OnInstallClicked);

  // Dot animation timer for the package label.
  m_DotTimer = new QTimer(this);
  m_DotTimer->setInterval(333);
  connect(m_DotTimer, &QTimer::timeout, this, &QmitkPipInstallDialog::OnDotTimer);

  // Installer connections.
  connect(m_Installer, &mitk::PipInstaller::VirtualEnvCreationStarted, this, &QmitkPipInstallDialog::OnVirtualEnvCreationStarted);
  connect(m_Installer, &mitk::PipInstaller::VirtualEnvCreationFinished, this, &QmitkPipInstallDialog::OnVirtualEnvCreationFinished);
  connect(m_Installer, &mitk::PipInstaller::PipUpgradeStarted, this, &QmitkPipInstallDialog::OnPipUpgradeStarted);
  connect(m_Installer, &mitk::PipInstaller::PipUpgradeFinished, this, &QmitkPipInstallDialog::OnPipUpgradeFinished);
  connect(m_Installer, &mitk::PipInstaller::ResolveStarted, this, &QmitkPipInstallDialog::OnResolveStarted);
  connect(m_Installer, &mitk::PipInstaller::ResolveFinished, this, &QmitkPipInstallDialog::OnResolveFinished);
  connect(m_Installer, &mitk::PipInstaller::PackageStatusChanged, this, &QmitkPipInstallDialog::OnPackageStatusChanged);
  connect(m_Installer, &mitk::PipInstaller::InstallFinished, this, &QmitkPipInstallDialog::OnInstallFinished);
  connect(m_Installer, &mitk::PipInstaller::ProgressChanged, this, &QmitkPipInstallDialog::OnProgressChanged);
  connect(m_Installer, &mitk::PipInstaller::ErrorOccurred, this, &QmitkPipInstallDialog::OnErrorOccurred);
}

QmitkPipInstallDialog::~QmitkPipInstallDialog()
{
}

void QmitkPipInstallDialog::closeEvent(QCloseEvent* event)
{
  if (m_IsInstalling && !ConfirmCancel())
    event->ignore();
}

void QmitkPipInstallDialog::reject()
{
  if (m_IsInstalling && !ConfirmCancel())
    return;

  QDialog::reject();
}

void QmitkPipInstallDialog::OnInstallClicked()
{
  // Total steps: 1 (venv, if needed) + 1 (prepare) + 2 per group (resolve + install).
  bool needsVirtualEnv = !m_Spec.venvName.empty() &&
                   !mitk::PythonHelper::VirtualEnvExists(m_Spec.venvName);
  m_TotalSteps = (needsVirtualEnv ? 1 : 0) + 1 + 2 * static_cast<int>(m_Spec.groups.size());
  m_CurrentStep = 0;

  m_Installer->SetInstallSpec(m_Spec);
  SetUiInstalling();
  m_Installer->StartResolveAndInstall();
}

// --- Installer event slots ---

void QmitkPipInstallDialog::OnVirtualEnvCreationStarted()
{
  m_CurrentStep = 1;
  SetStatus("Create virtual environment");
  m_Ui->progressBar->setRange(0, 0);
  m_Ui->progressBar->show();
}

void QmitkPipInstallDialog::OnVirtualEnvCreationFinished(bool /*success*/)
{
}

void QmitkPipInstallDialog::OnPipUpgradeStarted()
{
  ++m_CurrentStep;
  SetStatus("Prepare installation");
  m_Ui->progressBar->setRange(0, 0);
  m_Ui->progressBar->show();
}

void QmitkPipInstallDialog::OnPipUpgradeFinished(bool /*success*/)
{
}

void QmitkPipInstallDialog::OnResolveStarted()
{
  m_DotTimer->stop();
  ++m_CurrentStep;
  SetStatus("Resolve dependencies");
  m_Ui->progressBar->setRange(0, 0);
  m_Ui->packageLabel->hide();
}

void QmitkPipInstallDialog::OnResolveFinished(bool success, const std::vector<mitk::PipPackageInfo>& /*packages*/)
{
  if (!success)
  {
    m_DotTimer->stop();
    SetUiFinished(false);
    m_Ui->statusLabel->setText("Installation failed. Could not resolve dependencies.");
    m_Ui->packageLabel->hide();
    return;
  }
}

void QmitkPipInstallDialog::OnPackageStatusChanged(int index, const QString& name, mitk::PackageStatus status)
{
  if (status != mitk::PackageStatus::Installing)
    return;

  // Update the step label on the first package of each install phase.
  auto resolvedPackages = m_Installer->GetResolvedPackages();

  if (index == 0 || (index > 0 && resolvedPackages[index].group != resolvedPackages[index - 1].group))
  {
    ++m_CurrentStep;
    SetStatus("Install packages");
  }

  // Show name and version with animated dots.
  if (index < static_cast<int>(resolvedPackages.size()) && !resolvedPackages[index].version.empty())
    m_PackageLabelBaseText = QString("Installing %1 %2").arg(name, QString::fromStdString(resolvedPackages[index].version));
  else
    m_PackageLabelBaseText = QString("Installing %1").arg(name);

  m_DotCount = 0;
  m_Ui->packageLabel->setText(m_PackageLabelBaseText + ".");
  m_Ui->packageLabel->show();
  m_DotTimer->start();
}

void QmitkPipInstallDialog::OnInstallFinished(bool success)
{
  m_DotTimer->stop();
  SetUiFinished(success);

  if (success)
  {
    m_Ui->statusLabel->setText(QString::fromStdString(m_Spec.name) + " was installed successfully.");
    m_Ui->packageLabel->hide();
    this->accept();
  }
  else
  {
    m_Ui->statusLabel->setText("Installation failed. Please try again.");
    m_Ui->packageLabel->hide();
  }
}

void QmitkPipInstallDialog::OnProgressChanged(int current, int total)
{
  m_Ui->progressBar->setRange(0, total);
  m_Ui->progressBar->setValue(current);
}

void QmitkPipInstallDialog::OnErrorOccurred(const QString& message)
{
  m_DotTimer->stop();
  SetUiFinished(false);
  m_Ui->statusLabel->setText("Installation failed: " + message);
  m_Ui->packageLabel->hide();
}

void QmitkPipInstallDialog::OnDotTimer()
{
  m_DotCount = (m_DotCount + 1) % 3;
  m_Ui->packageLabel->setText(m_PackageLabelBaseText + QString(m_DotCount + 1, '.'));
}

// --- Private helpers ---

bool QmitkPipInstallDialog::ConfirmCancel()
{
  auto answer = QMessageBox::question(
    this,
    "Cancel installation",
    "<p>The installation is still in progress. "
    "Closing now will cancel the installation and remove all already installed packages.</p>"
    "<p>Cancel anyway?</p>");

  if (answer == QMessageBox::No)
    return false;

  m_Installer->Cancel();
  return true;
}

void QmitkPipInstallDialog::SetUiInstalling()
{
  m_IsInstalling = true;

  if (auto* button = m_Ui->buttonBox->button(QDialogButtonBox::Yes))
    button->setEnabled(false);

  m_Ui->statusLabel->show();
  m_Ui->packageLabel->setText("");
}

void QmitkPipInstallDialog::SetStatus(const QString& text)
{
  m_Ui->statusLabel->setText(
    QString("<h4>Step %1 of %2: %3</h4>").arg(m_CurrentStep).arg(m_TotalSteps).arg(text));
}

void QmitkPipInstallDialog::SetUiFinished(bool success)
{
  m_IsInstalling = false;

  if (success)
  {
    m_Ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    disconnect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QmitkPipInstallDialog::OnInstallClicked);
    connect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  }
  else
  {
    m_Ui->buttonBox->setStandardButtons(QDialogButtonBox::Retry | QDialogButtonBox::Cancel);
    disconnect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QmitkPipInstallDialog::OnInstallClicked);
  }

  m_Ui->buttonBox->setEnabled(true);
}
