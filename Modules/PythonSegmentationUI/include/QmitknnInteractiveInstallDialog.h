/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitknnInteractiveInstallDialog_h
#define QmitknnInteractiveInstallDialog_h

#include <QDialog>
#include <QProcess>

class QShowEvent;

namespace Ui
{
  class QmitknnInteractiveInstallDialog;
}

class QmitknnInteractiveInstallDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QmitknnInteractiveInstallDialog(QWidget* parent = nullptr);
  ~QmitknnInteractiveInstallDialog() override;

protected:
  enum class InstallStep
  {
    Upgrade_Pip,
    Install_PyTorch, // On Windows, we need a separate install step that uses pip --index-url.
    Install_nnInteractive
  };

  void OnYesClicked();
  void OnStandardOutputReady();
  void OnStandardErrorReady();
  void OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

  void AutoScrollToBottom();

private:
  Ui::QmitknnInteractiveInstallDialog* m_Ui;
  QProcess* m_Process;
  InstallStep m_InstallStep;
};

#endif
