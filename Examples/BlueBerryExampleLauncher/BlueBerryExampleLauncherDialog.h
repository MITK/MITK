/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BLUEBERRYEXAMPLELAUNCHERDIALOG_H
#define BLUEBERRYEXAMPLELAUNCHERDIALOG_H

#include <QDialog>

#include <QEventLoop>
#include <QStringList>

namespace Ui
{
  class BlueBerryExampleLauncherDialog;
}

class BlueBerryExampleLauncherDialog : public QDialog
{
  Q_OBJECT

public:
  explicit BlueBerryExampleLauncherDialog(QWidget *parent = nullptr);
  ~BlueBerryExampleLauncherDialog() override;

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
