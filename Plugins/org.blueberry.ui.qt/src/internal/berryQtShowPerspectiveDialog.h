/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
  explicit QtShowPerspectiveDialog(IPerspectiveRegistry* perspReg, QWidget *parent = 0);
  ~QtShowPerspectiveDialog();

  QString GetSelection() const;

private:

  Q_SLOT void setDescription(const QModelIndex& index);

  Ui::QtShowPerspectiveDialog *ui;
};

}

#endif // BERRYQTSHOWPERSPECTIVEDIALOG_H
