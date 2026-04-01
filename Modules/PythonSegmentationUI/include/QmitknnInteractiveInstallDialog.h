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
#include <memory>

class QShowEvent;

namespace Ui
{
  class QmitknnInteractiveInstallDialog;
}

/** \brief Dialog for installing the nnInteractive Python package and its dependencies.
 *
 * This dialog guides the user through a multi-step installation process:
 * -# Upgrade pip
 * -# Install PyTorch (with platform-specific CUDA index URL on Windows)
 * -# Install nnInteractive
 *
 * The dialog displays real-time output from the pip install processes and
 * provides advanced settings for customizing package versions and pip options.
 * On failure, the user can retry the installation from the beginning.
 *
 * The dialog accepts (QDialog::Accepted) only when all installation steps
 * complete successfully.
 *
 * \sa QmitknnInteractiveToolGUI, mitk::nnInteractiveTool
 */
class QmitknnInteractiveInstallDialog : public QDialog
{
  Q_OBJECT

public:
  /** \brief Constructs the install dialog.
   *
   * \param[in] parent The parent widget. Defaults to \c nullptr.
   */
  explicit QmitknnInteractiveInstallDialog(QWidget* parent = nullptr);

  /** \brief Destructor.
   */
  ~QmitknnInteractiveInstallDialog() override;

protected:
  /** \brief Specifies the sequential installation steps.
   */
  enum class InstallStep
  {
    Upgrade_Pip,          /**< \brief Upgrade pip to the latest version. */
    Install_PyTorch,      /**< \brief Install PyTorch. On Windows, uses a separate
                               step with pip \c --index-url for CUDA support. */
    Install_nnInteractive /**< \brief Install the nnInteractive package. */
  };

  /** \brief Toggles visibility of the advanced settings group box.
   *
   * \param[in] checked Whether advanced settings should be shown.
   */
  void OnShowAdvancedSettingsButtonClicked(bool checked);

  /** \brief Starts the installation process when the user clicks Yes.
   *
   * Disables the button box, shows an indeterminate progress bar, and
   * begins with the first installation step (upgrading pip).
   */
  void OnYesClicked();

  /** \brief Reads and displays standard output from the pip process.
   */
  void OnStandardOutputReady();

  /** \brief Reads and displays standard error from the pip process in red.
   */
  void OnStandardErrorReady();

  /** \brief Handles completion of an installation step.
   *
   * Advances to the next step on success, or offers a retry on failure.
   * Pip upgrade failures are tolerated (pip upgrade is optional).
   *
   * \param[in] exitCode The process exit code.
   * \param[in] exitStatus The process exit status.
   */
  void OnProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

  /** \brief Scrolls the output text area to the bottom.
   */
  void AutoScrollToBottom();

private:
  std::unique_ptr<Ui::QmitknnInteractiveInstallDialog> m_Ui;
  QProcess* m_Process;
  InstallStep m_InstallStep;
};

#endif
