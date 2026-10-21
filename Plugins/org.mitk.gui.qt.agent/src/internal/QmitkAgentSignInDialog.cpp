/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAgentSignInDialog.h"

#include <QmitkClaudeCodeProcess.h>

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

QmitkAgentSignInDialog::QmitkAgentSignInDialog(const QString& executable, Account account, QWidget* parent)
  : QDialog(parent),
    m_Process(new QProcess(this)),
    m_PageLabel(new QLabel(this)),
    m_CodeEdit(new QLineEdit(this)),
    m_ErrorLabel(new QLabel(this)),
    m_Buttons(new QDialogButtonBox(QDialogButtonBox::Cancel, this))
{
  const bool subscription = account == Account::Subscription;

  this->setWindowTitle(subscription ? "Sign In with a Claude Subscription" : "Sign In with an Anthropic Console Account");
  this->setMinimumWidth(480);

  auto* description = new QLabel("Sign in on the page that Claude Code opens in your browser. "
    "This dialog closes once Claude Code is signed in.", this);
  description->setWordWrap(true);

  m_PageLabel->setWordWrap(true);
  m_PageLabel->setOpenExternalLinks(true);
  m_PageLabel->hide();

  auto* codeLabel = new QLabel("If the page shows a code, paste it here:", this);

  // The default button, so that Enter in the code field continues instead of cancelling.
  auto* continueButton = new QPushButton("Continue", this);
  continueButton->setDefault(true);

  auto* codeLayout = new QHBoxLayout;
  codeLayout->addWidget(m_CodeEdit);
  codeLayout->addWidget(continueButton);

  m_ErrorLabel->setWordWrap(true);
  m_ErrorLabel->hide();

  auto* layout = new QVBoxLayout(this);
  layout->addWidget(description);
  layout->addWidget(m_PageLabel);
  layout->addWidget(codeLabel);
  layout->addLayout(codeLayout);
  layout->addWidget(m_ErrorLabel);
  layout->addWidget(m_Buttons);

  connect(continueButton, &QPushButton::clicked, this, &QmitkAgentSignInDialog::SubmitCode);
  connect(m_Buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

  connect(m_Process, &QProcess::readyReadStandardOutput, this, &QmitkAgentSignInDialog::OnOutput);
  connect(m_Process, &QProcess::finished, this, &QmitkAgentSignInDialog::OnFinished);

  // For example that a pasted code is not valid.
  connect(m_Process, &QProcess::readyReadStandardError, this, [this]() {
    m_ErrorOutput += m_Process->readAllStandardError();
    this->ShowError(QString::fromUtf8(m_ErrorOutput).trimmed());
  });

  connect(m_Process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
    if (error == QProcess::FailedToStart)
      this->ShowError(QString("Claude Code cannot be started: %1").arg(m_Process->errorString()));
  });

  m_Process->setProcessEnvironment(QmitkClaudeCodeProcess::GetEnvironment());
  m_Process->start(executable, { "auth", "login", subscription ? "--claudeai" : "--console" });
}

QmitkAgentSignInDialog::~QmitkAgentSignInDialog()
{
  // Deleting the process kills it, which must not reach this half-destroyed dialog.
  m_Process->disconnect(this);
}

void QmitkAgentSignInDialog::OnOutput()
{
  m_Output += m_Process->readAllStandardOutput();

  if (!m_PageLabel->isHidden())
    return;

  // The page in case the browser did not open. Where links are supported,
  // the address comes wrapped in escape sequences, which the pattern leaves
  // out. It must end before the output does, which may stop in the middle.
  static const QRegularExpression address(R"(https://[^\s\x07\x1b]+)");

  const auto output = QString::fromUtf8(m_Output);
  const auto match = address.match(output);

  if (!match.hasMatch() || match.capturedEnd() == output.size())
    return;

  m_PageLabel->setText(QString("If no browser opened, <a href=\"%1\">open the sign-in page</a>.").arg(match.captured().toHtmlEscaped()));
  m_PageLabel->show();
}

void QmitkAgentSignInDialog::OnFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  if (exitStatus == QProcess::NormalExit && exitCode == 0)
  {
    this->accept();
    return;
  }

  const auto error = QString::fromUtf8(m_ErrorOutput).trimmed();

  this->ShowError(error.isEmpty()
    ? QString("Claude Code ended before it was signed in (exit code %1).").arg(exitCode)
    : error);

  m_CodeEdit->setEnabled(false);
  m_Buttons->setStandardButtons(QDialogButtonBox::Close);
}

void QmitkAgentSignInDialog::SubmitCode()
{
  const auto code = m_CodeEdit->text().trimmed();

  if (code.isEmpty() || m_Process->state() != QProcess::Running)
    return;

  m_ErrorOutput.clear();
  m_ErrorLabel->hide();
  m_Process->write(code.toUtf8() + '\n');
  m_CodeEdit->clear();
}

void QmitkAgentSignInDialog::ShowError(const QString& error)
{
  m_ErrorLabel->setText(error);
  m_ErrorLabel->show();
}
