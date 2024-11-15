/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkToolInstallDialog.h"
#include <QMessageBox>

QTextEdit *QmitkToolInstallDialog::m_ConsoleOut = nullptr;

QmitkToolInstallDialog::QmitkToolInstallDialog(QWidget *parent)
  : QDialog(parent), m_IsInstalling(true), m_Ui(new Ui::QmitkToolInstallDialog)
{
  m_Ui->setupUi(this);
  this->setModal(true);
  this->setWindowTitle("TotalSegmentator Installer");
  m_Ui->closePushButton->setEnabled(false);
  m_ConsoleOut = new QTextEdit(this);
  m_ConsoleOut->setReadOnly(true);
  m_ConsoleOut->setMinimumHeight(50);
  m_Ui->gridLayout->addWidget(m_ConsoleOut, 1, 0, 1, 2);
  connect(m_Ui->closePushButton, &QPushButton::clicked, [this]() { this->close(); });
}

void QmitkToolInstallDialog::closeEvent(QCloseEvent *event)
{
  if (m_IsInstalling)
  {
    if (QMessageBox::No ==
        QMessageBox::question(nullptr,
                              "Cancel Installation",
                              QString("<p><b>WARNING:</b> Closing the installation process can cause undefined behaviour. "
                                      "Only close it, if you <i>really</i> have to.</p>"
                                      "<p>Close anyway?</p>"),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No))
    {
      event->ignore();
    }
    else
    {
      m_IsInstalling = false;
    }
  }
}
