/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKFILEWRITEROPTIONSDIALOG_H
#define QMITKFILEWRITEROPTIONSDIALOG_H

#include <mitkIOUtil.h>

#include <QDialog>

namespace Ui
{
  class QmitkFileWriterOptionsDialog;
}

class QmitkFileReaderWriterOptionsWidget;

class QmitkFileWriterOptionsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QmitkFileWriterOptionsDialog(mitk::IOUtil::SaveInfo &saveInfo, QWidget *parent = nullptr);
  ~QmitkFileWriterOptionsDialog() override;

  bool ReuseOptions() const;

  void accept() override;

private:
  Ui::QmitkFileWriterOptionsDialog *ui;
  mitk::IOUtil::SaveInfo &m_SaveInfo;
  std::vector<mitk::FileWriterSelector::Item> m_WriterItems;
};

#endif // QMITKFILEREADEROPTIONSDIALOG_H
