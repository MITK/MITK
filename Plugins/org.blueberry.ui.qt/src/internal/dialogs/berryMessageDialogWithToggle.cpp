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

#include "berryMessageDialogWithToggle.h"
#include "ui_berryMessageDialogWithToggle.h"

MessageDialogWithToggle::MessageDialogWithToggle(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::MessageDialogWithToggle)
{
  ui->setupUi(this);
}

MessageDialogWithToggle::~MessageDialogWithToggle()
{
  delete ui;
}
