/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAgentPreferencePage_h
#define QmitkAgentPreferencePage_h

#include <QmitkAgentSignInDialog.h>

#include <berryIQtPreferencePage.h>

#include <memory>

namespace Ui
{
  class QmitkAgentPreferencePage;
}

/**
 * \brief Settings of the agent: which Claude Code runs, how it signs in,
 *        and whether a subscription takes precedence over an API key.
 *
 * The credentials stay with Claude Code. The Agent view applies the
 * settings, and checks the sign-in again when Claude Code signed in here.
 */
class QmitkAgentPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkAgentPreferencePage();
  ~QmitkAgentPreferencePage() override;

  void Init(berry::IWorkbench::Pointer workbench) override;

  void CreateQtControl(QWidget* parent) override;
  QWidget* GetQtControl() const override;

  bool PerformOk() override;
  void PerformCancel() override;
  void Update() override;

private:
  /** \brief The executable that the settings on the page lead to; empty if none is found. */
  QString GetExecutable() const;

  /** \brief How Claude Code runs for the check and the sign-in, with the settings on the page, which may not be applied yet. */
  QmitkClaudeCodeProcess::Options GetOptions() const;

  /** \brief Shows how Claude Code signs in with the settings on the page, which may not be applied yet. */
  void UpdateStatus();

  void BrowseExecutable();
  void SignIn(QmitkAgentSignInDialog::Account account);

  std::unique_ptr<Ui::QmitkAgentPreferencePage> m_Ui;
  QWidget* m_Control;

  /** \brief Counts the queries of the sign-in, so that only the last one shows. */
  int m_StatusQueries;
};

#endif
