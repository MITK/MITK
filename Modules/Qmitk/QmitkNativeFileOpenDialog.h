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

#ifndef QMITKNATIVEFILEOPENDIALOG_H
#define QMITKNATIVEFILEOPENDIALOG_H

#include <QmitkExports.h>

#include <mitkBaseData.h>

#include <QFileDialog>
#include <QScopedPointer>

namespace mitk {
class DataStorage;
}

class QmitkNativeFileOpenDialogPrivate;

class QMITK_EXPORT QmitkNativeFileOpenDialog
{

public:

  QmitkNativeFileOpenDialog(QWidget* parent = NULL);
  ~QmitkNativeFileOpenDialog();

  void SetCaption(const QString& caption);
  void SetDir(const QString& dir);
  void SetOptions(QFileDialog::Options options);

  void Exec();

  QStringList GetSelectedFiles() const;

  QList<mitk::BaseData::Pointer> ReadBaseData(mitk::DataStorage* ds = NULL) const;

private:

  Q_DISABLE_COPY(QmitkNativeFileOpenDialog)

  QScopedPointer<QmitkNativeFileOpenDialogPrivate> d;
};

#endif // QMITKNATIVEFILEOPENDIALOG_H
