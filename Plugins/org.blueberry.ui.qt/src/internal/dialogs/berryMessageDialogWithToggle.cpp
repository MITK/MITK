/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
