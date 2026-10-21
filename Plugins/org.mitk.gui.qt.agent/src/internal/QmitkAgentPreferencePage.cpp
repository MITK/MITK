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

#include <ui_QmitkAgentPreferencePage.h>

#include <QmitkAgentSession.h>
#include <QmitkClaudeCodeProcess.h>

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QtConcurrent>

namespace
{
  /** The preferences of the Agent view, which applies them. */
  mitk::IPreferences* GetPreferences()
  {
    return mitk::CoreServices::GetPreferencesService()->GetSystemPreferences()->Node("/" + QmitkAgentView::VIEW_ID);
  }
}

QmitkAgentPreferencePage::QmitkAgentPreferencePage()
  : m_Ui(std::make_unique<Ui::QmitkAgentPreferencePage>()),
    m_Control(nullptr),
    m_StatusQueries(0)
{
}

QmitkAgentPreferencePage::~QmitkAgentPreferencePage() = default;

void QmitkAgentPreferencePage::Init(berry::IWorkbench::Pointer)
{
}

void QmitkAgentPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Control = new QWidget(parent);
  m_Ui->setupUi(m_Control);

  const auto found = QmitkClaudeCodeProcess::FindExecutable();
  m_Ui->executableEdit->setPlaceholderText(found.isEmpty() ? QString("Not found") : QDir::toNativeSeparators(found));

  connect(m_Ui->browseButton, &QPushButton::clicked, this, &QmitkAgentPreferencePage::BrowseExecutable);
  connect(m_Ui->executableEdit, &QLineEdit::editingFinished, this, &QmitkAgentPreferencePage::UpdateStatus);
  connect(m_Ui->ignoreApiKeyCheckBox, &QCheckBox::clicked, this, &QmitkAgentPreferencePage::UpdateStatus);

  connect(m_Ui->subscriptionButton, &QPushButton::clicked, this, [this]() {
    this->SignIn(QmitkAgentSignInDialog::Account::Subscription);
  });

  connect(m_Ui->consoleButton, &QPushButton::clicked, this, [this]() {
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
  preferences->Put("executable", QDir::fromNativeSeparators(m_Ui->executableEdit->text().trimmed()).toStdString());
  preferences->PutBool("ignoreApiKeyVariable", m_Ui->ignoreApiKeyCheckBox->isChecked());

  return true;
}

void QmitkAgentPreferencePage::PerformCancel()
{
}

void QmitkAgentPreferencePage::Update()
{
  const auto* preferences = GetPreferences();
  m_Ui->executableEdit->setText(QDir::toNativeSeparators(QString::fromStdString(preferences->Get("executable", ""))));
  m_Ui->ignoreApiKeyCheckBox->setChecked(preferences->GetBool("ignoreApiKeyVariable", false));

  this->UpdateStatus();
}

QString QmitkAgentPreferencePage::GetExecutable() const
{
  const auto executable = QDir::fromNativeSeparators(m_Ui->executableEdit->text().trimmed());

  return executable.isEmpty()
    ? QmitkClaudeCodeProcess::FindExecutable()
    : executable;
}

void QmitkAgentPreferencePage::UpdateStatus()
{
  const auto query = ++m_StatusQueries;
  const auto executable = this->GetExecutable();
  const bool usable = QmitkClaudeCodeProcess::IsUsableExecutable(executable);

  m_Ui->subscriptionButton->setEnabled(usable);
  m_Ui->consoleButton->setEnabled(usable);

  if (executable.isEmpty())
  {
    m_Ui->statusCard->SetMessage("Claude Code was not found. Install it, or set where it is.", QmitkInfoCard::Severity::Warning);
    return;
  }

  if (!usable)
  {
    m_Ui->statusCard->SetMessage("The agent cannot run this file as Claude Code. It needs the claude program itself.",
      QmitkInfoCard::Severity::Error);
    return;
  }

  m_Ui->statusCard->SetMessage("Checking how Claude Code signs in...");

  QmitkClaudeCodeProcess::Options options;
  options.Executable = executable;
  options.WorkingDirectory = QmitkAgentSession::GetWorkspaceDirectory();
  options.IgnoreApiKeyVariable = m_Ui->ignoreApiKeyCheckBox->isChecked();

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
      m_Ui->statusCard->SetMessage(status.Error.toHtmlEscaped(), QmitkInfoCard::Severity::Error);
    }
    else if (status.LoggedIn)
    {
      m_Ui->statusCard->SetMessage(QString("Signed in %1.").arg(status.Describe()).toHtmlEscaped());
    }
    else
    {
      m_Ui->statusCard->SetMessage("Not signed in.", QmitkInfoCard::Severity::Warning);
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

  m_Ui->executableEdit->setText(QDir::toNativeSeparators(file));
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
