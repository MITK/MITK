/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFileReaderOptionsDialog_h
#define QmitkFileReaderOptionsDialog_h

#include "mitkIOUtil.h"

#include <QDialog>

namespace Ui
{
  class QmitkFileReaderOptionsDialog;
}

class QmitkFileReaderWriterOptionsWidget;

class QmitkFileReaderOptionsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QmitkFileReaderOptionsDialog(mitk::IOUtil::LoadInfo &loadInfo, QWidget *parent = nullptr);
  ~QmitkFileReaderOptionsDialog() override;

  bool ReuseOptions() const;

  void accept() override;

protected slots:
  void SetCurrentReader(int index);

private:
  Ui::QmitkFileReaderOptionsDialog *ui;
  mitk::IOUtil::LoadInfo &m_LoadInfo;
  std::vector<mitk::FileReaderSelector::Item> m_ReaderItems;
};

#endif
