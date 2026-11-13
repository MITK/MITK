/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAgentSignInDialog_h
#define QmitkAgentSignInDialog_h

#include <MitkAgentUIExports.h>

#include <QmitkClaudeCodeProcess.h>

#include <QDialog>
#include <QProcess>

#include <memory>

namespace Ui
{
  class QmitkAgentSignInDialog;
}

/**
 * \brief Signs Claude Code in, like "claude auth login" in a terminal.
 *
 * Claude Code opens its sign-in page in the browser and waits for the page
 * to call it back. If the page shows a code instead, the user pastes it into
 * the dialog. The dialog is accepted once Claude Code is signed in, and the
 * credentials stay with Claude Code.
 */
class MITKAGENTUI_EXPORT QmitkAgentSignInDialog : public QDialog
{
public:
  enum class Account
  {
    /** \brief A Claude subscription, such as Pro, Max or Team. */
    Subscription,

    /** \brief An Anthropic Console account, billed per token. */
    Console
  };

  /** \param options How Claude Code runs, as for the check of the sign-in, so that signing in applies to what the agent runs. */
  QmitkAgentSignInDialog(const QmitkClaudeCodeProcess::Options& options, Account account, QWidget* parent);
  ~QmitkAgentSignInDialog() override;

private:
  void OnOutput();
  void OnFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void SubmitCode();
  void ShowError(const QString& error);

  /** \brief Shows the error and leaves only closing the dialog, as Claude Code does not run (anymore). */
  void ShowEnd(const QString& error);

  std::unique_ptr<Ui::QmitkAgentSignInDialog> m_Ui;
  QProcess* m_Process;
  QByteArray m_Output;
  QByteArray m_ErrorOutput;
};

#endif
