/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkAgentSignInDialog.h>
#include <ui_QmitkAgentSignInDialog.h>

#include <QmitkClaudeCodeProcess.h>

#include <QPushButton>
#include <QRegularExpression>

QmitkAgentSignInDialog::QmitkAgentSignInDialog(const QmitkClaudeCodeProcess::Options& options, Account account, QWidget* parent)
  : QDialog(parent),
    m_Ui(std::make_unique<Ui::QmitkAgentSignInDialog>()),
    m_Process(new QProcess(this))
{
  m_Ui->setupUi(this);

  const bool subscription = account == Account::Subscription;
  this->setWindowTitle(subscription ? "Sign In with a Claude Subscription" : "Sign In with an Anthropic Console Account");

  m_Ui->pageLabel->hide();
  m_Ui->errorLabel->hide();

  // The output of a process is no rich text, even where it looks like it.
  m_Ui->errorLabel->setTextFormat(Qt::PlainText);

  connect(m_Ui->continueButton, &QPushButton::clicked, this, &QmitkAgentSignInDialog::SubmitCode);
  connect(m_Ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  connect(m_Process, &QProcess::readyReadStandardOutput, this, &QmitkAgentSignInDialog::OnOutput);
  connect(m_Process, &QProcess::finished, this, &QmitkAgentSignInDialog::OnFinished);

  // For example that a pasted code is not valid.
  connect(m_Process, &QProcess::readyReadStandardError, this, [this]() {
    m_ErrorOutput += m_Process->readAllStandardError();
    this->ShowError(QString::fromUtf8(m_ErrorOutput).trimmed());
  });

  connect(m_Process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
    // No finished() follows a failed start.
    if (error == QProcess::FailedToStart)
      this->ShowEnd(QString("Claude Code cannot be started: %1").arg(m_Process->errorString()));
  });

  QmitkClaudeCodeProcess::SetUpAuthCommand(*m_Process, options, { "login", subscription ? "--claudeai" : "--console" });
  m_Process->start();
}

QmitkAgentSignInDialog::~QmitkAgentSignInDialog()
{
  // Killing the process must not reach this half-destroyed dialog.
  m_Process->disconnect(this);

  // Still running after Cancel. Deleted while it runs, the process would warn
  // about it and wait far longer.
  if (m_Process->state() != QProcess::NotRunning)
  {
    m_Process->kill();
    m_Process->waitForFinished(3000);
  }
}

void QmitkAgentSignInDialog::OnOutput()
{
  m_Output += m_Process->readAllStandardOutput();

  if (!m_Ui->pageLabel->isHidden())
    return;

  // The page in case the browser did not open. Where links are supported,
  // the address comes wrapped in escape sequences, which the pattern leaves
  // out. It must end before the output does, which may stop in the middle.
  static const QRegularExpression address(R"(https://[^\s\x07\x1b]+)");

  const auto output = QString::fromUtf8(m_Output);
  const auto match = address.match(output);

  if (!match.hasMatch() || match.capturedEnd() == output.size())
    return;

  m_Ui->pageLabel->setText(QString("If no browser opened, <a href=\"%1\">open the sign-in page</a>.").arg(match.captured().toHtmlEscaped()));
  m_Ui->pageLabel->show();
}

void QmitkAgentSignInDialog::OnFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitStatus == QProcess::NormalExit && exitCode == 0)
  {
    this->accept();
    return;
  }

  const auto error = QString::fromUtf8(m_ErrorOutput).trimmed();

  this->ShowEnd(error.isEmpty()
    ? QString("Claude Code ended before it was signed in (exit code %1).").arg(exitCode)
    : error);
}

void QmitkAgentSignInDialog::SubmitCode()
{
  const auto code = m_Ui->codeEdit->text().trimmed();

  if (code.isEmpty() || m_Process->state() != QProcess::Running)
    return;

  m_ErrorOutput.clear();
  m_Ui->errorLabel->hide();
  m_Process->write(code.toUtf8() + '\n');
  m_Ui->codeEdit->clear();
}

void QmitkAgentSignInDialog::ShowError(const QString& error)
{
  m_Ui->errorLabel->setText(error);
  m_Ui->errorLabel->show();
}

void QmitkAgentSignInDialog::ShowEnd(const QString& error)
{
  this->ShowError(error);

  m_Ui->codeEdit->setEnabled(false);
  m_Ui->continueButton->setEnabled(false);
  m_Ui->buttonBox->setStandardButtons(QDialogButtonBox::Close);
}
