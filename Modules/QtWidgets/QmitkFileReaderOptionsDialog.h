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

#ifndef QMITKFILEREADEROPTIONSDIALOG_H
#define QMITKFILEREADEROPTIONSDIALOG_H

#include <QDialog>

namespace mitk {
class IFileReader;
}

namespace Ui {
class QmitkFileReaderOptionsDialog;
}

class QmitkFileReaderWriterOptionsWidget;

class QmitkFileReaderOptionsDialog : public QDialog
{
  Q_OBJECT

public:

  explicit QmitkFileReaderOptionsDialog(const QString& filePath,
                                        const QStringList& labels,
                                        const QList<mitk::IFileReader*>& readers,
                                        QWidget *parent = 0);
  ~QmitkFileReaderOptionsDialog();

  mitk::IFileReader* GetReader() const;

  bool ReuseOptions() const;

  virtual void accept();

private:

  Ui::QmitkFileReaderOptionsDialog* ui;
  const QList<mitk::IFileReader*> m_Readers;
};

#endif // QMITKFILEREADEROPTIONSDIALOG_H
