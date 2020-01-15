/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkUSControlsDopplerWidget.h"
#include "ui_QmitkUSControlsDopplerWidget.h"

QmitkUSControlsDopplerWidget::QmitkUSControlsDopplerWidget(mitk::USControlInterfaceDoppler::Pointer controlInterface, QWidget *parent)
  : QWidget(parent), ui(new Ui::QmitkUSControlsDopplerWidget),
    m_ControlInterface(controlInterface)
{
  ui->setupUi(this);

  if ( ! m_ControlInterface )
  {
    return;
  }
}

QmitkUSControlsDopplerWidget::~QmitkUSControlsDopplerWidget()
{
  delete ui;
}
