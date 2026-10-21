/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAgentSignInDialog_h
#define QmitkAgentSignInDialog_h

#include <QDialog>
#include <QProcess>

class QDialogButtonBox;
class QLabel;
class QLineEdit;

/**
 * \brief Signs Claude Code in, like "claude auth login" in a terminal.
 *
 * Claude Code opens its sign-in page in the browser and waits for the page
 * to call it back. If the page shows a code instead, the user pastes it into
 * the dialog. The dialog is accepted once Claude Code is signed in, and the
 * credentials stay with Claude Code.
 */
class QmitkAgentSignInDialog : public QDialog
{
public:
  enum class Account
  {
    /** \brief A Claude subscription, such as Pro, Max or Team. */
    Subscription,

    /** \brief An Anthropic Console account, billed per token. */
    Console
  };

  QmitkAgentSignInDialog(const QString& executable, Account account, QWidget* parent);
  ~QmitkAgentSignInDialog() override;

private:
  void OnOutput();
  void OnFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void SubmitCode();
  void ShowError(const QString& error);

  QProcess* m_Process;
  QByteArray m_Output;
  QByteArray m_ErrorOutput;
  QLabel* m_PageLabel;
  QLineEdit* m_CodeEdit;
  QLabel* m_ErrorLabel;
  QDialogButtonBox* m_Buttons;
};

#endif
