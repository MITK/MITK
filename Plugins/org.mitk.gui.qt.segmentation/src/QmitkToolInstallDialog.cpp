/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkToolInstallDialog.h"
#include <ui_QmitkToolInstallDialog.h>

#include <QCloseEvent>
#include <QMessageBox>

QPlainTextEdit* QmitkToolInstallDialog::s_ConsoleOutput = nullptr;

QObject* QmitkToolInstallDialog::GetConsoleOutput()
{
  return s_ConsoleOutput;
}

QmitkToolInstallDialog::QmitkToolInstallDialog(QWidget* parent)
  : QDialog(parent),
    m_IsInstalling(true),
    m_Ui(new Ui::QmitkToolInstallDialog)
{
  m_Ui->setupUi(this);
  s_ConsoleOutput = m_Ui->consoleOutput;

  connect(m_Ui->closePushButton, &QPushButton::clicked, this, &QmitkToolInstallDialog::close);
}

QmitkToolInstallDialog::~QmitkToolInstallDialog()
{
  s_ConsoleOutput = nullptr;
}

void QmitkToolInstallDialog::closeEvent(QCloseEvent* event)
{
  if (m_IsInstalling)
  {
    const QString title = "Cancel installation";
    const QString message = "<p><b>WARNING:</b> Closing the installation process can cause undefined behaviour. "
                            "Only close it, if you <i>really</i> have to.</p>"
                            "<p>Close anyway?</p>";

    if (QMessageBox::No == QMessageBox::question(this, title, message))
    {
      event->ignore();
    }
    else
    {
      m_IsInstalling = false;
    }
  }
}

void QmitkToolInstallDialog::reject()
{
}
