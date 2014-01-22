/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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