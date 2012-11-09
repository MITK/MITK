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


#ifndef APPFRAMEWORKDEMODIALOG_H
#define APPFRAMEWORKDEMODIALOG_H

#include <QDialog>

#include <QEventLoop>
#include <QStringList>

namespace Ui {
class AppFrameworkDemoDialog;
}

class AppFrameworkDemoDialog : public QDialog
{
  Q_OBJECT

public:
  explicit AppFrameworkDemoDialog(QWidget *parent = 0);
  ~AppFrameworkDemoDialog();

  QString getDemoConfiguration();

private:

  Q_SLOT void configurationSelected();
  Q_SLOT void dialogCanceled();

  Q_SLOT void selectionChanged(int row);

private:

  Ui::AppFrameworkDemoDialog *ui;
  QStringList provisioningFiles;
  QEventLoop eventLoop;
};

#endif // APPFRAMEWORKDEMODIALOG_H
