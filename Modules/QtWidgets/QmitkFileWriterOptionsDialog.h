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

#ifndef QMITKFILEWRITEROPTIONSDIALOG_H
#define QMITKFILEWRITEROPTIONSDIALOG_H

#include <QDialog>

namespace mitk {
class IFileWriter;
}

namespace Ui {
class QmitkFileWriterOptionsDialog;
}

class QmitkFileReaderWriterOptionsWidget;

class QmitkFileWriterOptionsDialog : public QDialog
{
  Q_OBJECT

public:

  explicit QmitkFileWriterOptionsDialog(const QString& filePath,
                                        const QStringList& labels,
                                        const QList<mitk::IFileWriter*>& writers,
                                        QWidget *parent = 0);
  ~QmitkFileWriterOptionsDialog();

  mitk::IFileWriter* GetWriter() const;

  bool ReuseOptions() const;

  virtual void accept();

private:

  Ui::QmitkFileWriterOptionsDialog* ui;
  const QList<mitk::IFileWriter*> m_Writers;
};

#endif // QMITKFILEREADEROPTIONSDIALOG_H
