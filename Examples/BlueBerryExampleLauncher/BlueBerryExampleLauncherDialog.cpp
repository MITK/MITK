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


#include "BlueBerryExampleLauncherDialog.h"
#include "ui_BlueBerryExampleLauncherDialog.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QDir>
#include <QPushButton>

BlueBerryExampleLauncherDialog::BlueBerryExampleLauncherDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::BlueBerryExampleLauncherDialog)
{
  ui->setupUi(this);

  connect(this, SIGNAL(accepted()), this, SLOT(configurationSelected()));
  connect(this, SIGNAL(rejected()), this, SLOT(dialogCanceled()));
  connect(ui->appList, SIGNAL(currentRowChanged(int)), this, SLOT(selectionChanged(int)));

  // Get all BlueBerryExampleLauncher_*.provisioning files
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

    descriptions.push_back(QString());
    QString descriptionFileName = provFile.left(provFile.lastIndexOf('.')) + ".txt";
    QFile descriptionFile(appDir.filePath(descriptionFileName));

    if (descriptionFile.exists())
    {
      if (descriptionFile.open(QIODevice::ReadOnly))
      {
        descriptions.back() = descriptionFile.readAll();
      }
    }
  }

  if (ui->appList->currentRow() == -1)
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

BlueBerryExampleLauncherDialog::~BlueBerryExampleLauncherDialog()
{
  delete ui;
}

QString BlueBerryExampleLauncherDialog::getDemoConfiguration()
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

void BlueBerryExampleLauncherDialog::configurationSelected()
{
  eventLoop.exit(0);
}

void BlueBerryExampleLauncherDialog::dialogCanceled()
{
  eventLoop.exit(1);
}

void BlueBerryExampleLauncherDialog::selectionChanged(int row)
{
  if (row > -1)
  {
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    ui->description->setHtml(this->descriptions[row]);
  }
  else
  {
    ui->description->clear();
  }
}
