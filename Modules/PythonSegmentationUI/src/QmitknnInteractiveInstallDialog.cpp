/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitknnInteractiveInstallDialog.h>
#include <ui_QmitknnInteractiveInstallDialog.h>

#include <mitkLogMacros.h>
#include <mitkPythonHelper.h>

#include <QMessageBox>
#include <QProcess>
#include <QScrollBar>

namespace
{
  // With PyTorch v2.9.0, nnInteractive has a 4x performance regression.
  // Starting with PyTorch v2.9.1, support for the GeForce 10-series GPUs is dropped.
  constexpr auto TORCH = "torch>=2.8.0,<2.9.0";

  constexpr auto TORCH_VISION = "torchvision>=0.23.0,<1.0.0";
  constexpr auto NNINTERACTIVE = "nninteractive>=1.1.2,<2.0.0";

#if defined(_WIN32)
  // Starting with CUDA v12.9 we get the following error on our lowest
  // supported GPU architecture (e.g. GeForce 10 Series):
  //   torch.AcceleratorError: CUDA error: no kernel image is available
  //   for exec
  constexpr auto CUDA_INDEX_URL = "https://download.pytorch.org/whl/cu128";
#endif
}

using Self = QmitknnInteractiveInstallDialog;

QmitknnInteractiveInstallDialog::QmitknnInteractiveInstallDialog(QWidget* parent)
  : QDialog(parent),
    m_Ui(new Ui::QmitknnInteractiveInstallDialog),
    m_Process(new QProcess(this)),
    m_InstallStep(InstallStep::Upgrade_Pip)
{
  m_Ui->setupUi(this);

  m_Ui->advancedGroupBox->hide();

  m_Ui->torchLineEdit->setText(TORCH);
  m_Ui->torchvisionLineEdit->setText(TORCH_VISION);
  m_Ui->nnInteractiveLineEdit->setText(NNINTERACTIVE);

#if defined(_WIN32)
  m_Ui->indexUrlLineEdit->setText(CUDA_INDEX_URL);
#else
  m_Ui->indexUrlLabel->hide();
  m_Ui->indexUrlLineEdit->hide();
#endif

  connect(m_Ui->advancedButton, &QPushButton::clicked, this, &Self::OnShowAdvancedSettingsButtonClicked);

  disconnect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &Self::OnYesClicked);

  connect(m_Process, &QProcess::readyReadStandardOutput, this, &Self::OnStandardOutputReady);
  connect(m_Process, &QProcess::readyReadStandardError, this, &Self::OnStandardErrorReady);
  connect(m_Process, &QProcess::finished, this, &Self::OnProcessFinished);
}

QmitknnInteractiveInstallDialog::~QmitknnInteractiveInstallDialog()
{
}

void QmitknnInteractiveInstallDialog::OnShowAdvancedSettingsButtonClicked(bool checked)
{
  m_Ui->advancedGroupBox->setVisible(checked);
  m_Ui->advancedButton->setText(checked
    ? "Hide advanced settings"
    : "Show advanced settings");
}

void QmitknnInteractiveInstallDialog::OnYesClicked()
{
  m_Ui->buttonBox->setEnabled(false);
  m_Ui->progressBar->setRange(0, 0);
  m_Ui->textEdit->clear();

  QStringList args = { "-m", "pip", "install", "--upgrade", "pip" };
  m_Process->start(QString::fromStdString(mitk::PythonHelper::GetExecutablePath().string()), args);
}

void QmitknnInteractiveInstallDialog::OnStandardOutputReady()
{
  auto output = QString::fromLocal8Bit(m_Process->readAllStandardOutput());
  output.replace('\n', "<br>");

  m_Ui->textEdit->moveCursor(QTextCursor::End);
  m_Ui->textEdit->insertHtml(QString("<span style=\"font-family: 'Courier New', monospace\">%1</span>").arg(output));

  this->AutoScrollToBottom();
}

void QmitknnInteractiveInstallDialog::OnStandardErrorReady()
{
  auto errorOutput = QString::fromLocal8Bit(m_Process->readAllStandardError());
  errorOutput.replace('\n', "<br>");

  m_Ui->textEdit->moveCursor(QTextCursor::End);
  m_Ui->textEdit->insertHtml(QString("<span style=\"font-family: 'Courier New', monospace; color: red\">%1</span>").arg(errorOutput));

  this->AutoScrollToBottom();
}

void QmitknnInteractiveInstallDialog::AutoScrollToBottom()
{
  if (auto scrollBar = m_Ui->textEdit->verticalScrollBar(); scrollBar != nullptr)
    scrollBar->setValue(scrollBar->maximum());
}

void QmitknnInteractiveInstallDialog::OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  // If any step fails, offer to restart from the beginning.
  // Exception: allow pip upgrade errors, since upgrading pip is optional.

  if (m_InstallStep != InstallStep::Upgrade_Pip && (exitStatus != QProcess::NormalExit || exitCode != 0))
  {
    m_InstallStep = InstallStep::Upgrade_Pip;

    m_Ui->progressBar->setRange(0, 100);
    m_Ui->progressBar->setValue(0);

    m_Ui->textEdit->insertHtml("<hr><h3 style=\"color: red\">nnInteractive could not be installed</h3>");

    m_Ui->buttonBox->setStandardButtons(QDialogButtonBox::Retry | QDialogButtonBox::Cancel);
    m_Ui->buttonBox->setEnabled(true);

    this->AutoScrollToBottom();
    return;
  }

  // Current step completed successfully - determine the next step.

  if (m_InstallStep == InstallStep::Upgrade_Pip)
  {
    m_InstallStep = InstallStep::Install_PyTorch;
  }
  else if (m_InstallStep == InstallStep::Install_PyTorch)
  {
    m_InstallStep = InstallStep::Install_nnInteractive;
  }
  else
  {
    // All steps completed successfully.

    m_Ui->progressBar->setRange(0, 100);
    m_Ui->progressBar->setValue(100);

    m_Ui->textEdit->insertHtml("<hr><h3>nnInteractive was installed successfully</h3>");

    m_Ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    disconnect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &Self::OnYesClicked);
    connect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    m_Ui->buttonBox->setEnabled(true);

    this->AutoScrollToBottom();

    if (m_Ui->autoCloseCheckBox->isChecked())
      this->accept();

    return;
  }

  // More steps remain - run the next one.

  if (m_InstallStep == InstallStep::Install_PyTorch)
  {
    auto torch = m_Ui->torchLineEdit->text();

    if (torch.isEmpty())
      torch = TORCH;

    auto torchvision = m_Ui->torchvisionLineEdit->text();

    if (torchvision.isEmpty())
      torchvision = TORCH_VISION;

    QStringList args = { "-m", "pip", "install", torch, torchvision };

#if defined(_WIN32)
    auto indexUrl = m_Ui->indexUrlLineEdit->text();

    if (indexUrl.isEmpty())
      indexUrl = CUDA_INDEX_URL;

    args.append({ "--index-url", indexUrl });
#endif

    if (m_Ui->noCacheDirCheckBox->isChecked())
      args.append("--no-cache-dir");

    m_Process->start(QString::fromStdString(mitk::PythonHelper::GetExecutablePath().string()), args);
  }
  else // InstallStep::Install_nnInteractive
  {
    auto nnInteractive = m_Ui->nnInteractiveLineEdit->text();

    if (nnInteractive.isEmpty())
      nnInteractive = NNINTERACTIVE;

    QStringList args = { "-m", "pip", "install", nnInteractive };

    if (m_Ui->noCacheDirCheckBox->isChecked())
      args.append("--no-cache-dir");

    m_Process->start(QString::fromStdString(mitk::PythonHelper::GetExecutablePath().string()), args);
  }
}
