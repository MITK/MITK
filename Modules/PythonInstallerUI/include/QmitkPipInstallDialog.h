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
 * Shows a package list with per-package status indicators (pending, installing,
 * installed, failed), an overall progress bar, and a collapsible raw output
 * area for advanced users.
 *
 * Usage:
 * \code
 * mitk::PipInstallSpec spec;
 * spec.name = "nnInteractive";
 * spec.groups.append({ {"torch>=2.8.0,<2.9.0"}, cudaIndexUrl, {} });
 * spec.groups.append({ {"nninteractive>=1.1.2,<2.0.0"}, {}, {} });
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

  void OnPipUpgradeStarted();
  void OnPipUpgradeFinished(bool success);
  void OnResolveStarted();
  void OnResolveFinished(bool success, const QList<mitk::PipPackageInfo>& packages);
  void OnPackageStatusChanged(int index, const QString& name, mitk::PackageStatus status);
  void OnInstallFinished(bool success);
  void OnOutputReceived(const QString& text, bool isError);
  void OnProgressChanged(int current, int total);
  void OnErrorOccurred(const QString& message);

  void OnShowAdvancedSettingsClicked(bool checked);
  void OnShowDetailsClicked(bool checked);

private:
  void SetPackageStatus(int index, mitk::PackageStatus status);
  void SetUiInstalling();
  void SetUiFinished(bool success);
  void AutoScrollOutput();
  void BuildAdvancedSettings();

  std::unique_ptr<Ui::QmitkPipInstallDialog> m_Ui;
  mitk::PipInstaller* m_Installer;
  mitk::PipInstallSpec m_Spec;
  bool m_IsInstalling = false;
};

#endif
