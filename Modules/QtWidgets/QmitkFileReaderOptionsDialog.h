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

#include "mitkIOUtil.h"

#include <QDialog>

namespace Ui {
class QmitkFileReaderOptionsDialog;
}

class QmitkFileReaderWriterOptionsWidget;

class QmitkFileReaderOptionsDialog : public QDialog
{
  Q_OBJECT

public:

  explicit QmitkFileReaderOptionsDialog(mitk::IOUtil::LoadInfo& loadInfo,
                                        QWidget *parent = 0);
  ~QmitkFileReaderOptionsDialog();

  bool ReuseOptions() const;

  virtual void accept();

private:

  Ui::QmitkFileReaderOptionsDialog* ui;
  mitk::IOUtil::LoadInfo& m_LoadInfo;
  std::vector<mitk::FileReaderSelector::Item> m_ReaderItems;
};

#endif // QMITKFILEREADEROPTIONSDIALOG_H
