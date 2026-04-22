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

#include <QmitkPipInstallAdvancedDialog.h>

#include <QCloseEvent>
#include <QEventLoop>
#include <QMessageBox>
#include <QPushButton>
#include <QShowEvent>
#include <QTextCursor>

QmitkPipInstallDialog::QmitkPipInstallDialog(const mitk::PipInstallSpec& spec, QWidget* parent)
  : QDialog(parent),
    m_Ui(std::make_unique<Ui::QmitkPipInstallDialog>()),
    m_Installer(new mitk::PipInstaller(this)),
    m_Spec(spec)
{
  m_Ui->setupUi(this);

  auto name = QString::fromStdString(spec.name);

  this->setWindowTitle(name + " Installer");

  m_Ui->descriptionLabel->setTextFormat(Qt::RichText);
  m_Ui->descriptionLabel->setText(
    QString("<h3>Do you want to install %1?</h3>"
            "<p>This may take a while depending on your internet connection.</p>")
      .arg(name.toHtmlEscaped()));

  m_Ui->statusLabel->setTextFormat(Qt::RichText);

  // Hide progress elements until installation starts.
  m_Ui->statusLabel->hide();
  m_Ui->progressBar->hide();
  m_Ui->packageLabel->hide();

  // Rename the Ok button to "Install <name>".
  if (auto* button = m_Ui->buttonBox->button(QDialogButtonBox::Ok))
    button->setText("Install " + name);

  // Wire the Install (Ok) button to our slot instead of the default accept.
  disconnect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QmitkPipInstallDialog::OnInstallClicked);

  connect(m_Ui->advancedSettingsButton, &QPushButton::clicked, this, &QmitkPipInstallDialog::OnAdvancedSettingsClicked);
  connect(m_Ui->detailsButton, &QPushButton::clicked, this, &QmitkPipInstallDialog::OnToggleDetailsClicked);

  // Dot animation timer for the package label.
  m_DotTimer = new QTimer(this);
  m_DotTimer->setInterval(333);
  connect(m_DotTimer, &QTimer::timeout, this, &QmitkPipInstallDialog::OnDotTimer);

  // Installer connections.
  connect(m_Installer, &mitk::PipInstaller::VirtualEnvCreationStarted, this, &QmitkPipInstallDialog::OnVirtualEnvCreationStarted);
  connect(m_Installer, &mitk::PipInstaller::PipUpgradeStarted, this, &QmitkPipInstallDialog::OnPipUpgradeStarted);
  connect(m_Installer, &mitk::PipInstaller::ResolveStarted, this, &QmitkPipInstallDialog::OnResolveStarted);
  connect(m_Installer, &mitk::PipInstaller::PackageStatusChanged, this, &QmitkPipInstallDialog::OnPackageStatusChanged);
  connect(m_Installer, &mitk::PipInstaller::ModelDownloadStarted, this, &QmitkPipInstallDialog::OnModelDownloadStarted);
  connect(m_Installer, &mitk::PipInstaller::InstallFinished, this, &QmitkPipInstallDialog::OnInstallFinished);
  connect(m_Installer, &mitk::PipInstaller::ProgressChanged, this, &QmitkPipInstallDialog::OnProgressChanged);
  connect(m_Installer, &mitk::PipInstaller::ErrorOccurred, this, &QmitkPipInstallDialog::OnErrorOccurred);
  connect(m_Installer, &mitk::PipInstaller::OutputReceived, this, &QmitkPipInstallDialog::OnOutputReceived);

  // Compact / expanded heights are captured on first show (see showEvent),
  // after Qt has run layout/DPI/font metrics; querying height() here would
  // return the .ui-declared value rather than the laid-out height.
}

void QmitkPipInstallDialog::showEvent(QShowEvent* event)
{
  QDialog::showEvent(event);

  if (!m_HeightsCaptured)
  {
    m_CompactHeight = this->height();
    m_ExpandedHeight = m_CompactHeight + 200;
    this->setFixedHeight(m_CompactHeight);
    m_HeightsCaptured = true;
  }
}

QmitkPipInstallDialog::~QmitkPipInstallDialog()
{
}

void QmitkPipInstallDialog::closeEvent(QCloseEvent* event)
{
  if (m_Installer->IsRunning() && !this->ConfirmCancel())
  {
    event->ignore();
    return;
  }

  // The user is dismissing the dialog. If a previous attempt failed and left
  // a partial venv behind, clean it up. No-op after a successful install or
  // after Cancel already did the cleanup.
  m_Installer->AbandonInstall();
}

void QmitkPipInstallDialog::reject()
{
  if (m_Installer->IsRunning() && !this->ConfirmCancel())
    return;

  // The user is dismissing the dialog. If a previous attempt failed and left
  // a partial venv behind, clean it up. No-op after a successful install or
  // after Cancel already did the cleanup.
  m_Installer->AbandonInstall();

  QDialog::reject();
}

void QmitkPipInstallDialog::OnAdvancedSettingsClicked()
{
  QmitkPipInstallAdvancedDialog dialog(m_Spec, this);

  if (dialog.exec() == QDialog::Accepted)
    m_Spec = dialog.GetInstallSpec();
}

void QmitkPipInstallDialog::OnInstallClicked()
{
  // Total steps: 1 (venv, if needed) + 1 (pip upgrade, if requested) + 2 per
  // group + 1 per Hugging Face model download. The installer appends a
  // synthetic huggingface_hub install group when Hugging Face downloads are
  // present, so count that extra group here too.
  bool needsVirtualEnv = !m_Spec.venvName.empty() &&
                   !mitk::PythonHelper::VirtualEnvExists(m_Spec.venvName);
  int effectiveGroups = static_cast<int>(m_Spec.groups.size()) +
                        (m_Spec.huggingFaceDownloads.empty() ? 0 : 1);
  m_TotalSteps = (needsVirtualEnv ? 1 : 0) + (m_Spec.upgradePipFirst ? 1 : 0) +
                 2 * effectiveGroups +
                 static_cast<int>(m_Spec.huggingFaceDownloads.size());
  m_CurrentStep = 0;

  m_Installer->SetInstallSpec(m_Spec);
  this->SetUiInstalling();
  m_Installer->StartInstall();
}

void QmitkPipInstallDialog::OnToggleDetailsClicked()
{
  this->SetDetailsVisible(!m_Ui->detailsView->isVisible());
}

void QmitkPipInstallDialog::OnVirtualEnvCreationStarted()
{
  ++m_CurrentStep;
  this->SetStatus("Create virtual environment");
  m_Ui->progressBar->setRange(0, 0);
  m_Ui->progressBar->show();
}

void QmitkPipInstallDialog::OnPipUpgradeStarted()
{
  ++m_CurrentStep;
  this->SetStatus("Prepare installation");
  m_Ui->progressBar->setRange(0, 0);
  m_Ui->progressBar->show();
}

void QmitkPipInstallDialog::OnResolveStarted()
{
  m_DotTimer->stop();
  ++m_CurrentStep;
  this->SetStatus("Resolve dependencies");
  m_Ui->progressBar->setRange(0, 0);
  m_Ui->packageLabel->hide();
}

void QmitkPipInstallDialog::OnPackageStatusChanged(int index, const QString& name, mitk::PackageStatus status)
{
  if (status != mitk::PackageStatus::Installing)
    return;

  // Update the step label on the first package of each install phase.
  const auto& resolvedPackages = m_Installer->GetResolvedPackages();

  if (index == 0 || (index > 0 && resolvedPackages[index].group != resolvedPackages[index - 1].group))
  {
    ++m_CurrentStep;
    this->SetStatus("Install packages");
  }

  // Show name and version with animated dots.
  if (index < static_cast<int>(resolvedPackages.size()) && !resolvedPackages[index].version.empty())
    m_PackageLabelBaseText = QString("Installing %1 %2").arg(name, QString::fromStdString(resolvedPackages[index].version));
  else
    m_PackageLabelBaseText = QString("Installing %1").arg(name);

  m_DotCount = 0;
  m_Ui->packageLabel->setText(m_PackageLabelBaseText);
  m_Ui->packageLabel->show();
  m_DotTimer->start();
}

void QmitkPipInstallDialog::OnModelDownloadStarted(const QString& displayName)
{
  m_DotTimer->stop();
  ++m_CurrentStep;
  this->SetStatus("Download model weights");

  // Indeterminate while the download runs. tqdm progress from huggingface_hub
  // shows up in the details view via OutputReceived.
  m_Ui->progressBar->setRange(0, 0);
  m_Ui->progressBar->show();

  m_PackageLabelBaseText = QString("Downloading %1").arg(displayName);
  m_DotCount = 0;
  m_Ui->packageLabel->setText(m_PackageLabelBaseText);
  m_Ui->packageLabel->show();
  m_DotTimer->start();
}

void QmitkPipInstallDialog::OnInstallFinished(bool success)
{
  m_DotTimer->stop();

  // While ConfirmCancel is spinning a local event loop, it owns the teardown
  // of the dialog. Don't touch the UI here - the failure-state widgets would
  // briefly flash before the dialog is rejected.
  if (m_IsCancelling)
    return;

  this->SetUiFinished(success);

  if (success)
  {
    this->SetTerminalStatus(QString::fromStdString(m_Spec.name) + " was installed successfully.");
    m_Ui->packageLabel->hide();
    // The last phase may have left the bar in indeterminate (spinning) mode
    // (HF downloads use setRange(0, 0)), so hide it once we're done.
    m_Ui->progressBar->hide();

    if (m_Ui->autoCloseCheckBox->isChecked())
    {
      this->accept();
    }
    else
    {
      // User opted out of auto-close so they can inspect the log. Offer the
      // details toggle for a successful run too - the raw pip output may still
      // be useful (deprecation warnings, resolved versions, etc.).
      this->OfferDetails();
    }
  }
  else
  {
    this->SetTerminalStatus("Installation failed. Please try again.");
    m_Ui->packageLabel->hide();
    m_Ui->progressBar->hide();
    this->OfferDetails();
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
  this->SetUiFinished(false);
  this->SetTerminalStatus("Installation failed: " + message);
  m_Ui->packageLabel->hide();
  m_Ui->progressBar->hide();
  this->OfferDetails();
}

void QmitkPipInstallDialog::OnOutputReceived(const QString& text, bool /*isError*/)
{
  // Accumulate raw pip output so we can offer it as diagnostic detail on failure.
  // Use insertPlainText (not appendPlainText) because pip output arrives in
  // arbitrary chunks; appendPlainText would insert a newline per chunk and
  // mangle multi-line messages.
  m_Ui->detailsView->moveCursor(QTextCursor::End);
  m_Ui->detailsView->insertPlainText(text);
  m_Ui->detailsView->moveCursor(QTextCursor::End);
}

void QmitkPipInstallDialog::OnDotTimer()
{
  m_Ui->packageLabel->setText(m_PackageLabelBaseText + QString(m_DotCount, '.'));
  m_DotCount = (m_DotCount + 1) % 4;
}

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

  m_IsCancelling = true;

  // Show that a cancel is in progress and prevent further interaction while
  // pip is being killed and the venv torn down.
  m_DotTimer->stop();
  this->SetTerminalStatus("Cancelling installation...");
  m_Ui->packageLabel->hide();
  m_Ui->progressBar->hide();
  m_Ui->buttonBox->setEnabled(false);

  // Spin a local event loop so the dialog stays alive until FinalizeCancel
  // has run (it removes the venv that we created) and emits InstallFinished.
  // Without this, the dialog would close as soon as ConfirmCancel returned
  // and the engine would be destroyed before OnProcessFinished could fire.
  //
  // Safety cap: if the pip kill or the venv removal wedges (AV holding files
  // open, zombie subprocess on Windows, etc.), quit the loop after 30 s so
  // the dialog can close instead of locking forever. A legitimate
  // fs::remove_all is comfortably inside that envelope even on a slow disk.
  // Queued connection so the synchronous Cancel() path (no QProcess running)
  // that emits InstallFinished directly from inside Cancel() still reaches
  // the loop: the queued slot is posted as an event and delivered once
  // loop.exec() starts spinning.
  QEventLoop loop;
  connect(m_Installer, &mitk::PipInstaller::InstallFinished, &loop, &QEventLoop::quit, Qt::QueuedConnection);
  QTimer::singleShot(30000, &loop, &QEventLoop::quit);
  m_Installer->Cancel();
  loop.exec();

  return true;
}

void QmitkPipInstallDialog::SetUiInstalling()
{
  m_Ui->advancedSettingsButton->hide();

  // A previous attempt may have left the details visible / accumulated output.
  // Reset the details area so a retry starts clean. SetDetailsVisible(false)
  // restores the spacer policy, hides the view, resets the button label, and
  // shrinks the dialog back to the compact height.
  m_Ui->detailsButton->hide();
  this->SetDetailsVisible(false);
  m_Ui->detailsView->clear();

  // Reset the cancellation latch so a retry after a cancelled-then-abandoned
  // attempt is not silently swallowed by the m_IsCancelling gate in
  // OnInstallFinished. Also re-enable the button box that ConfirmCancel
  // disables during teardown.
  m_IsCancelling = false;
  m_Ui->buttonBox->setEnabled(true);

  if (auto* button = m_Ui->buttonBox->button(QDialogButtonBox::Ok))
    button->setEnabled(false);

  // Allow SetTerminalStatus to publish a new terminal message for this run.
  m_TerminalStatusSet = false;

  m_Ui->statusLabel->show();
  m_Ui->packageLabel->clear();
}

void QmitkPipInstallDialog::SetStatus(const QString& text)
{
  m_Ui->statusLabel->setText(
    QString("<h4>Step %1 of %2: %3</h4>").arg(m_CurrentStep).arg(m_TotalSteps).arg(text));
}

void QmitkPipInstallDialog::SetTerminalStatus(const QString& text)
{
  // The first specific terminal message wins. This lets OnErrorOccurred
  // publish a precise cause (e.g. "Python executable not found") without
  // the subsequent OnInstallFinished(false) overwriting it with the
  // generic fallback. Reset in SetUiInstalling so retries can publish a
  // new terminal message.
  if (m_TerminalStatusSet)
    return;

  m_Ui->statusLabel->setText(text);
  m_TerminalStatusSet = true;
}

void QmitkPipInstallDialog::SetUiFinished(bool success)
{
  // Clear both possible accepted-slot bindings before rewiring so repeated
  // fail/retry cycles don't accumulate duplicate connections. Otherwise the
  // constructor's connect to OnInstallClicked survives the failure path and
  // SetUiFinished(false) would stack a second connect on top of it, causing
  // a single Retry click to fire OnInstallClicked twice.
  disconnect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QmitkPipInstallDialog::OnInstallClicked);
  disconnect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

  if (success)
  {
    m_Ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    connect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  }
  else
  {
    m_Ui->buttonBox->setStandardButtons(QDialogButtonBox::Retry | QDialogButtonBox::Cancel);
    connect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QmitkPipInstallDialog::OnInstallClicked);
  }

  m_Ui->buttonBox->setEnabled(true);
}

void QmitkPipInstallDialog::OfferDetails()
{
  // Only meaningful if pip actually produced output. If it didn't, hide the
  // toggle so the dialog stays minimal.
  if (m_Ui->detailsView->toPlainText().isEmpty())
    return;

  m_Ui->detailsButton->show();
}

void QmitkPipInstallDialog::SetDetailsVisible(bool show)
{
  // When the details view is visible, it should consume the extra vertical
  // space; the spacer becomes Fixed(0, 0). When the details view is hidden,
  // the spacer takes over and pushes the button row to the bottom.
  m_Ui->verticalSpacer->changeSize(
    0, 0, QSizePolicy::Minimum, show ? QSizePolicy::Fixed : QSizePolicy::Expanding);
  m_Ui->mainLayout->invalidate();

  m_Ui->detailsView->setVisible(show);
  m_Ui->detailsButton->setText(show ? "Hide details" : "Show details");

  this->setFixedHeight(show ? m_ExpandedHeight : m_CompactHeight);
}
