/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYMESSAGEDIALOGWITHTOGGLE_H
#define BERRYMESSAGEDIALOGWITHTOGGLE_H

#include <QDialog>

namespace Ui {
class MessageDialogWithToggle;
}

class MessageDialogWithToggle : public QDialog
{
  Q_OBJECT

public:
  explicit MessageDialogWithToggle(QWidget *parent = 0);
  ~MessageDialogWithToggle();

private:
  Ui::MessageDialogWithToggle *ui;
};

#endif // BERRYMESSAGEDIALOGWITHTOGGLE_H
