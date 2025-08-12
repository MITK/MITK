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
  constexpr auto TORCH = "torch==2.7.1";
  constexpr auto TORCH_VISION = "torchvision==0.22.1";
  constexpr auto NNINTERACTIVE = "nninteractive>=1.1.2,<2.0.0";
  constexpr auto CUDA_INDEX_URL = "https://download.pytorch.org/whl/cu118";
}

using Self = QmitknnInteractiveInstallDialog;

QmitknnInteractiveInstallDialog::QmitknnInteractiveInstallDialog(QWidget* parent)
  : QDialog(parent),
    m_Ui(new Ui::QmitknnInteractiveInstallDialog),
    m_Process(new QProcess(this)),
    m_InstallStep(InstallStep::Upgrade_Pip)
{
  m_Ui->setupUi(this);

  disconnect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &Self::OnYesClicked);

  connect(m_Process, &QProcess::readyReadStandardOutput, this, &Self::OnStandardOutputReady);
  connect(m_Process, &QProcess::readyReadStandardError, this, &Self::OnStandardErrorReady);
  connect(m_Process, &QProcess::finished, this, &Self::OnProcessFinished);
}

QmitknnInteractiveInstallDialog::~QmitknnInteractiveInstallDialog()
{
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
#if defined(_WIN32)
    // On Windows, installing PyTorch is a separate step to allow passing --index-url to pip.
    m_InstallStep = InstallStep::Install_PyTorch;
#else
    m_InstallStep = InstallStep::Install_nnInteractive;
#endif
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
    QStringList args = { "-m", "pip", "install", TORCH, TORCH_VISION, "--index-url", CUDA_INDEX_URL };
    m_Process->start(QString::fromStdString(mitk::PythonHelper::GetExecutablePath().string()), args);
  }
  else // InstallStep::Install_nnInteractive
  {
    QStringList args = { "-m", "pip", "install", NNINTERACTIVE };
    m_Process->start(QString::fromStdString(mitk::PythonHelper::GetExecutablePath().string()), args);
  }
}
