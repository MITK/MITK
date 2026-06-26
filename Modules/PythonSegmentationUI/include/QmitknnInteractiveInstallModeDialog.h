/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitknnInteractiveInstallModeDialog_h
#define QmitknnInteractiveInstallModeDialog_h

#include <QDialog>

#include <memory>

namespace Ui
{
  class QmitknnInteractiveInstallModeDialog;
}

/** \brief Asks whether to install nnInteractive fully or client-only.
 *
 * Shown right before the pip install dialog when nnInteractive is not yet
 * installed. The choice determines what is installed:
 *
 * - \c Full: the full nnInteractive package (PyTorch, the local inference
 *   backend, and a model checkpoint), enabling both local and remote sessions.
 * - \c ClientOnly: the lightweight \c nninteractive-client package (no PyTorch),
 *   enabling remote sessions only. Much smaller and faster to install.
 *
 * Switching from client-only to full later is done by uninstalling and
 * reinitializing, so the choice is presented up front rather than as a setting.
 *
 * \sa QmitknnInteractiveToolGUI, QmitkPipInstallDialog
 */
class QmitknnInteractiveInstallModeDialog : public QDialog
{
  Q_OBJECT

public:
  /** \brief The install mode chosen by the user. */
  enum class Mode
  {
    Full,      /**< \brief Local and remote inference (installs PyTorch and a model checkpoint). */
    ClientOnly /**< \brief Remote sessions only (installs the lightweight nninteractive-client). */
  };

  explicit QmitknnInteractiveInstallModeDialog(QWidget* parent = nullptr);
  ~QmitknnInteractiveInstallModeDialog() override;

  /** \brief Returns the selected mode. Only meaningful after the dialog was accepted. */
  Mode SelectedMode() const;

private:
  std::unique_ptr<Ui::QmitknnInteractiveInstallModeDialog> m_Ui;
};

#endif
