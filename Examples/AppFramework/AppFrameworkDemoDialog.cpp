/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#include "AppFrameworkDemoDialog.h"
#include "ui_AppFrameworkDemoDialog.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QDir>
#include <QPushButton>

AppFrameworkDemoDialog::AppFrameworkDemoDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::AppFrameworkDemoDialog)
{
  ui->setupUi(this);

  connect(this, SIGNAL(accepted()), this, SLOT(configurationSelected()));
  connect(this, SIGNAL(rejected()), this, SLOT(dialogCanceled()));
  connect(ui->appList, SIGNAL(currentRowChanged(int)), this, SLOT(selectionChanged(int)));

  // Get all AppFrameworkDemo_*.provisioning files
  QDir appDir(QApplication::applicationDirPath());
#ifdef CMAKE_INTDIR
  appDir.cdUp();
#endif

  provisioningFiles =
      appDir.entryList(QStringList(QApplication::applicationName() + "_*.provisioning"),
                                   QDir::Files | QDir::Readable, QDir::Name);

  foreach(QString provFile, provisioningFiles)
  {
    int startIndex = provFile.indexOf('_');
    int endIndex = provFile.lastIndexOf('.');
    ui->appList->addItem(provFile.mid(startIndex+1, endIndex-startIndex-1));
  }

  if (ui->appList->currentRow() == -1)
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

AppFrameworkDemoDialog::~AppFrameworkDemoDialog()
{
  delete ui;
}

QString AppFrameworkDemoDialog::getDemoConfiguration()
{
  this->show();

  if (eventLoop.exec() == 0)
  {
    QDir appDir = QCoreApplication::applicationDirPath();
    #ifdef CMAKE_INTDIR
      appDir.cdUp();
    #endif
    
    return appDir.filePath(provisioningFiles[ui->appList->currentRow()]);
  }

  return QString();
}

void AppFrameworkDemoDialog::configurationSelected()
{
  eventLoop.exit(0);
}

void AppFrameworkDemoDialog::dialogCanceled()
{
  eventLoop.exit(1);
}

void AppFrameworkDemoDialog::selectionChanged(int row)
{
  if (row > -1)
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}
