/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitknnInteractiveInstallModeDialog.h>
#include <ui_QmitknnInteractiveInstallModeDialog.h>

#include <QDialogButtonBox>
#include <QPushButton>

QmitknnInteractiveInstallModeDialog::QmitknnInteractiveInstallModeDialog(QWidget* parent)
  : QDialog(parent),
    m_Ui(std::make_unique<Ui::QmitknnInteractiveInstallModeDialog>())
{
  m_Ui->setupUi(this);

#if defined(__APPLE__)
  // On macOS local inference has no GPU acceleration (Apple Silicon runs on the
  // CPU; Intel does not support local at all), so default to the lightweight
  // client-only install that offloads to a remote server. The .ui defaults to
  // Full for other platforms.
  m_Ui->clientOnlyRadioButton->setChecked(true);

#if !defined(__aarch64__)
  // Intel Macs cannot run nnInteractive locally, so do not offer the full (local)
  // install at all here; only client-only/remote is possible. This prevents a Full
  // choice from downloading multi-GB PyTorch and a checkpoint that could never be
  // used. (Apple Silicon keeps Full available: CPU-only inference is still useful
  // when no server is around.)
  m_Ui->fullRadioButton->setEnabled(false);
  m_Ui->fullDescriptionLabel->setEnabled(false);
#endif
#endif

  if (auto* okButton = m_Ui->buttonBox->button(QDialogButtonBox::Ok))
    okButton->setText("Continue");

  connect(m_Ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_Ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QmitknnInteractiveInstallModeDialog::~QmitknnInteractiveInstallModeDialog()
{
}

QmitknnInteractiveInstallModeDialog::Mode QmitknnInteractiveInstallModeDialog::SelectedMode() const
{
  return m_Ui->clientOnlyRadioButton->isChecked() ? Mode::ClientOnly : Mode::Full;
}
