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


#ifndef BLUEBERRYEXAMPLELAUNCHERDIALOG_H
#define BLUEBERRYEXAMPLELAUNCHERDIALOG_H

#include <QDialog>

#include <QEventLoop>
#include <QStringList>

namespace Ui {
class BlueBerryExampleLauncherDialog;
}

class BlueBerryExampleLauncherDialog : public QDialog
{
  Q_OBJECT

public:
  explicit BlueBerryExampleLauncherDialog(QWidget *parent = 0);
  ~BlueBerryExampleLauncherDialog();

  QString getDemoConfiguration();

private:

  Q_SLOT void configurationSelected();
  Q_SLOT void dialogCanceled();

  Q_SLOT void selectionChanged(int row);

private:

  Ui::BlueBerryExampleLauncherDialog *ui;
  QStringList provisioningFiles;
  QList<QString> descriptions;
  QEventLoop eventLoop;
};

#endif // BLUEBERRYEXAMPLELAUNCHERDIALOG_H
