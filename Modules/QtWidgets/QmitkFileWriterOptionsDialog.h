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

#include <mitkIOUtil.h>

#include <QDialog>

namespace Ui {
class QmitkFileWriterOptionsDialog;
}

class QmitkFileReaderWriterOptionsWidget;

class QmitkFileWriterOptionsDialog : public QDialog
{
  Q_OBJECT

public:

  explicit QmitkFileWriterOptionsDialog(mitk::IOUtil::SaveInfo& saveInfo, QWidget *parent = 0);
  ~QmitkFileWriterOptionsDialog();

  bool ReuseOptions() const;

  virtual void accept();

private:

  Ui::QmitkFileWriterOptionsDialog* ui;
  mitk::IOUtil::SaveInfo& m_SaveInfo;
  std::vector<mitk::FileWriterSelector::Item> m_WriterItems;
};

#endif // QMITKFILEREADEROPTIONSDIALOG_H
