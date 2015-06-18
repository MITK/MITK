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
