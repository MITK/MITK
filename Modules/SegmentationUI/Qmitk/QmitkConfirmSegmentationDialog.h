/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKCONFIRMSEGMENTATIONDIALOG_H
#define QMITKCONFIRMSEGMENTATIONDIALOG_H

#include <QDialog>

namespace Ui
{
  class QmitkConfirmSegmentationDialog;
}

class QmitkConfirmSegmentationDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QmitkConfirmSegmentationDialog(QWidget *parent = nullptr);
  ~QmitkConfirmSegmentationDialog() override;

  void SetSegmentationName(QString name);

  enum
  {
    OVERWRITE_SEGMENTATION,
    CREATE_NEW_SEGMENTATION,
    CANCEL_SEGMENTATION
  };

protected slots:

  void OnOverwriteExistingSegmentation();

  void OnCreateNewSegmentation();

  void OnCancelSegmentation();

private:
  Ui::QmitkConfirmSegmentationDialog *m_Controls;
};

#endif // QMITKCONFIRMSEGMENTATIONDIALOG_H
