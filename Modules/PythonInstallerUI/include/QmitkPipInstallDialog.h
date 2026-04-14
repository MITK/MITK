/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPipInstallDialog_h
#define QmitkPipInstallDialog_h

#include <MitkPythonInstallerUIExports.h>

#include <mitkPipPackageInfo.h>

#include <QDialog>
#include <QTimer>
#include <memory>

class QCloseEvent;

namespace Ui
{
  class QmitkPipInstallDialog;
}

namespace mitk
{
  class PipInstaller;
}

/** \brief Reusable dialog for installing Python packages via pip.
 *
 * Presents a simple, non-technical installation dialog with a progress bar
 * and per-package status label. Designed for end users who do not need to
 * see raw pip output or manage package versions.
 *
 * Usage:
 * \code
 * mitk::PipInstallSpec spec;
 * spec.name = "nnInteractive";
 * spec.groups.push_back({ {"torch>=2.8.0,<2.9.0"}, cudaIndexUrl, {} });
 * spec.groups.push_back({ {"nninteractive>=1.1.2,<2.0.0"}, {}, {} });
 *
 * QmitkPipInstallDialog dialog(spec);
 * if (dialog.exec() == QDialog::Accepted) { ... }
 * \endcode
 *
 * \sa mitk::PipInstaller, mitk::PipInstallSpec
 */
class MITKPYTHONINSTALLERUI_EXPORT QmitkPipInstallDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QmitkPipInstallDialog(const mitk::PipInstallSpec& spec, QWidget* parent = nullptr);
  ~QmitkPipInstallDialog() override;

protected:
  void closeEvent(QCloseEvent* event) override;
  void reject() override;

private slots:
  void OnInstallClicked();
  void OnAdvancedSettingsClicked();

  void OnVirtualEnvCreationStarted();
  void OnVirtualEnvCreationFinished(bool success);
  void OnPipUpgradeStarted();
  void OnPipUpgradeFinished(bool success);
  void OnResolveStarted();
  void OnResolveFinished(bool success, const std::vector<mitk::PipPackageInfo>& packages);
  void OnPackageStatusChanged(int index, const QString& name, mitk::PackageStatus status);
  void OnInstallFinished(bool success);
  void OnProgressChanged(int current, int total);
  void OnErrorOccurred(const QString& message);
  void OnDotTimer();

private:
  bool ConfirmCancel();
  void SetUiInstalling();
  void SetUiFinished(bool success);

  void SetStatus(const QString& text);

  std::unique_ptr<Ui::QmitkPipInstallDialog> m_Ui;
  mitk::PipInstaller* m_Installer;
  mitk::PipInstallSpec m_Spec;
  QTimer* m_DotTimer;
  QString m_PackageLabelBaseText;
  int m_DotCount = 0;
  bool m_IsInstalling = false;
  int m_CurrentStep = 0;
  int m_TotalSteps = 0;
};

#endif
