/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYQTSHOWPERSPECTIVEDIALOG_H
#define BERRYQTSHOWPERSPECTIVEDIALOG_H

#include <QDialog>

class QModelIndex;

namespace Ui {
class QtShowPerspectiveDialog;
}

namespace berry {

struct IPerspectiveRegistry;

class QtShowPerspectiveDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QtShowPerspectiveDialog(IPerspectiveRegistry* perspReg, QWidget *parent = nullptr);
  ~QtShowPerspectiveDialog() override;

  QString GetSelection() const;

private:

  Q_SLOT void setDescription(const QModelIndex& index);

  Ui::QtShowPerspectiveDialog *ui;
};

}

#endif // BERRYQTSHOWPERSPECTIVEDIALOG_H
