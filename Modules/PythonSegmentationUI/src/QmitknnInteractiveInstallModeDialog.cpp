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
