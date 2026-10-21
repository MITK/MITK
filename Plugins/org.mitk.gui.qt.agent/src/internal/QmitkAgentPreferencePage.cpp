/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAgentPreferencePage.h"
#include "QmitkAgentPluginActivator.h"
#include "QmitkAgentView.h"

#include <QmitkClaudeCodeProcess.h>
#include <QmitkInfoCard.h>

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>

#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtConcurrent>

namespace
{
  /** The preferences of the Agent view, which applies them. */
  mitk::IPreferences* GetPreferences()
  {
    return mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node("/" + QmitkAgentView::VIEW_ID);
  }
}

void QmitkAgentPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkAgentPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);
  auto* layout = new QVBoxLayout(m_Control);

  auto* introLabel = new QLabel("The agent is your own installation of Claude Code. It signs in like Claude Code does, "
    "and the Workbench never sees your credentials.");
  introLabel->setWordWrap(true);
  layout->addWidget(introLabel);

  const auto found = QmitkClaudeCodeProcess::FindExecutable();

  m_ExecutableEdit = new QLineEdit;
  m_ExecutableEdit->setPlaceholderText(found.isEmpty() ? QString("Not found") : QDir::toNativeSeparators(found));
  m_ExecutableEdit->setToolTip("Leave empty to look for Claude Code on the PATH and in ~/.local/bin, where its installer puts it.");

  auto* browseButton = new QPushButton("Browse...");

  auto* executableLayout = new QHBoxLayout;
  executableLayout->addWidget(new QLabel("Claude Code:"));
  executableLayout->addWidget(m_ExecutableEdit);
  executableLayout->addWidget(browseButton);
  layout->addLayout(executableLayout);

  auto* signInGroup = new QGroupBox("Sign-in");
  auto* signInLayout = new QVBoxLayout(signInGroup);
  signInLayout->setSpacing(10);

  m_StatusCard = new QmitkInfoCard;
  signInLayout->addWidget(m_StatusCard);

  // As tall as the main buttons of views.
  m_SubscriptionButton = new QPushButton("Sign In with a Claude Subscription...");
  m_SubscriptionButton->setMinimumHeight(40);

  m_ConsoleButton = new QPushButton("Sign In with an Anthropic Console Account...");
  m_ConsoleButton->setMinimumHeight(40);
  m_ConsoleButton->setToolTip("Without a subscription, billed per token.");

  auto* buttonsLayout = new QHBoxLayout;
  buttonsLayout->setSpacing(signInLayout->spacing());
  buttonsLayout->addWidget(m_SubscriptionButton, 1);
  buttonsLayout->addWidget(m_ConsoleButton, 1);
  signInLayout->addLayout(buttonsLayout);

  m_IgnoreApiKeyCheckBox = new QCheckBox("Use a Claude subscription even if ANTHROPIC_API_KEY is set");
  m_IgnoreApiKeyCheckBox->setToolTip("Claude Code prefers an API key in the environment variable ANTHROPIC_API_KEY over a "
    "Claude subscription, and the key is billed per token. With this, the agent does not get the variable.");
  signInLayout->addWidget(m_IgnoreApiKeyCheckBox);

  layout->addWidget(signInGroup);
  layout->addStretch();

  connect(browseButton, &QPushButton::clicked, this, &QmitkAgentPreferencePage::BrowseExecutable);
  connect(m_ExecutableEdit, &QLineEdit::editingFinished, this, &QmitkAgentPreferencePage::UpdateStatus);
  connect(m_IgnoreApiKeyCheckBox, &QCheckBox::clicked, this, &QmitkAgentPreferencePage::UpdateStatus);

  connect(m_SubscriptionButton, &QPushButton::clicked, this, [this]() {
    this->SignIn(QmitkAgentSignInDialog::Account::Subscription);
  });

  connect(m_ConsoleButton, &QPushButton::clicked, this, [this]() {
    this->SignIn(QmitkAgentSignInDialog::Account::Console);
  });

  this->Update();
}

QWidget* QmitkAgentPreferencePage::GetQtControl() const
{
  return m_Control;
}

bool QmitkAgentPreferencePage::PerformOk()
{
  auto* preferences = GetPreferences();
  preferences->Put("executable", QDir::fromNativeSeparators(m_ExecutableEdit->text().trimmed()).toStdString());
  preferences->PutBool("ignoreApiKeyVariable", m_IgnoreApiKeyCheckBox->isChecked());

  return true;
}

void QmitkAgentPreferencePage::PerformCancel()
{
}

void QmitkAgentPreferencePage::Update()
{
  const auto* preferences = GetPreferences();
  m_ExecutableEdit->setText(QDir::toNativeSeparators(QString::fromStdString(preferences->Get("executable", ""))));
  m_IgnoreApiKeyCheckBox->setChecked(preferences->GetBool("ignoreApiKeyVariable", false));

  this->UpdateStatus();
}

QString QmitkAgentPreferencePage::GetExecutable() const
{
  const auto executable = QDir::fromNativeSeparators(m_ExecutableEdit->text().trimmed());

  return executable.isEmpty()
    ? QmitkClaudeCodeProcess::FindExecutable()
    : executable;
}

void QmitkAgentPreferencePage::UpdateStatus()
{
  const auto query = ++m_StatusQueries;
  const auto executable = this->GetExecutable();
  const bool usable = QmitkClaudeCodeProcess::IsUsableExecutable(executable);

  m_SubscriptionButton->setEnabled(usable);
  m_ConsoleButton->setEnabled(usable);

  if (executable.isEmpty())
  {
    m_StatusCard->SetMessage("Claude Code was not found. Install it, or set where it is.", QmitkInfoCard::Severity::Warning);
    return;
  }

  if (!usable)
  {
    m_StatusCard->SetMessage("The agent cannot run this file as Claude Code. It needs the claude program itself.",
      QmitkInfoCard::Severity::Error);
    return;
  }

  m_StatusCard->SetMessage("Checking how Claude Code signs in...");

  QmitkClaudeCodeProcess::Options options;
  options.Executable = executable;
  options.WorkingDirectory = QmitkAgentView::GetWorkspaceDirectory();
  options.IgnoreApiKeyVariable = m_IgnoreApiKeyCheckBox->isChecked();

  // With the control, which the preferences dialog deletes before the page.
  auto* watcher = new QFutureWatcher<QmitkClaudeCodeProcess::AuthStatus>(m_Control);

  connect(watcher, &QFutureWatcher<QmitkClaudeCodeProcess::AuthStatus>::finished, m_Control, [this, watcher, query]() {
    const auto status = watcher->result();
    watcher->deleteLater();

    // The settings on the page changed meanwhile, and a newer query follows.
    if (query != m_StatusQueries)
      return;

    if (!status.Error.isEmpty())
    {
      m_StatusCard->SetMessage(status.Error.toHtmlEscaped(), QmitkInfoCard::Severity::Error);
    }
    else if (status.LoggedIn)
    {
      m_StatusCard->SetMessage(QString("Signed in %1.").arg(status.Describe()).toHtmlEscaped());
    }
    else
    {
      m_StatusCard->SetMessage("Not signed in.", QmitkInfoCard::Severity::Warning);
    }
  });

  watcher->setFuture(QtConcurrent::run(&QmitkClaudeCodeProcess::QueryAuthStatus, options));
}

void QmitkAgentPreferencePage::BrowseExecutable()
{
#ifdef Q_OS_WIN
  const QString filter = "Programs (*.exe)";
#else
  const QString filter;
#endif

  const auto current = this->GetExecutable();
  const auto directory = current.isEmpty() ? QDir::homePath() : QFileInfo(current).path();
  const auto file = QFileDialog::getOpenFileName(m_Control, "Claude Code", directory, filter);

  if (file.isEmpty())
    return;

  m_ExecutableEdit->setText(QDir::toNativeSeparators(file));
  this->UpdateStatus();
}

void QmitkAgentPreferencePage::SignIn(QmitkAgentSignInDialog::Account account)
{
  QmitkAgentSignInDialog dialog(this->GetExecutable(), account, m_Control);

  if (dialog.exec() != QDialog::Accepted)
    return;

  this->UpdateStatus();

  // Signing in takes effect at once, not only with OK.
  if (auto* activator = QmitkAgentPluginActivator::GetInstance(); activator != nullptr)
    emit activator->ClaudeCodeSignedIn();
}
