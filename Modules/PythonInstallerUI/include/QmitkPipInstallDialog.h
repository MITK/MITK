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
class QShowEvent;
class QmitkPipInstaller;

namespace Ui
{
  class QmitkPipInstallDialog;
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
 *
 * mitk::PipInstallGroup torchGroup;
 * torchGroup.requirements = { "torch>=2.8.0,<2.9.0" };
 * torchGroup.indexUrl = "https://download.pytorch.org/whl/cu128";
 * spec.groups.push_back(torchGroup);
 *
 * mitk::PipInstallGroup appGroup;
 * appGroup.requirements = { "nninteractive>=1.1.2,<2.0.0" };
 * spec.groups.push_back(appGroup);
 *
 * QmitkPipInstallDialog dialog(spec, parent);
 * if (dialog.exec() == QDialog::Accepted) { ... }
 * \endcode
 *
 * \sa QmitkPipInstaller, mitk::PipInstallSpec
 */
class MITKPYTHONINSTALLERUI_EXPORT QmitkPipInstallDialog : public QDialog
{
  Q_OBJECT

public:
  /** \brief Whether the dialog presents a fresh install or an in-place update.
   *
   * Only affects user-facing wording (window title, description, button labels,
   * success message) and hides the Advanced-settings button in Update mode. The
   * underlying install engine is identical; an update is driven by passing
   * \c --upgrade in the spec's group extra pip arguments.
   */
  enum class Mode { Install, Update };

  explicit QmitkPipInstallDialog(const mitk::PipInstallSpec& spec, QWidget* parent = nullptr, Mode mode = Mode::Install);
  ~QmitkPipInstallDialog() override;

protected:
  void closeEvent(QCloseEvent* event) override;
  void showEvent(QShowEvent* event) override;
  void reject() override;

private slots:
  void OnInstallClicked();
  void OnAdvancedSettingsClicked();
  void OnToggleDetailsClicked();

  void OnVirtualEnvCreationStarted();
  void OnPipUpgradeStarted();
  void OnResolveStarted();
  void OnPackageStatusChanged(int index, const QString& name, mitk::PackageStatus status);
  void OnPostInstallStepStarted(const QString& displayName);
  void OnInstallFinished(bool success);
  void OnProgressChanged(int current, int total);
  void OnErrorOccurred(const QString& message);
  void OnOutputReceived(const QString& text, bool isError);
  void OnDotTimer();

private:
  bool ConfirmCancel();
  void SetUiInstalling();
  void SetUiFinished(bool success);
  void OfferDetails();
  void SetDetailsVisible(bool show);

  void SetStatus(const QString& text);
  void SetTerminalStatus(const QString& text);

  std::unique_ptr<Ui::QmitkPipInstallDialog> m_Ui;
  QmitkPipInstaller* m_Installer = nullptr;
  mitk::PipInstallSpec m_Spec;
  Mode m_Mode = Mode::Install;
  QTimer* m_DotTimer = nullptr;
  QString m_PackageLabelBaseText;
  int m_DotCount = 0;
  int m_CurrentStep = 0;
  int m_TotalSteps = 0;
  int m_CompactHeight = 0;
  int m_ExpandedHeight = 0;
  bool m_HeightsCaptured = false;
  bool m_IsCancelling = false;
  bool m_TerminalStatusSet = false;
};

#endif
