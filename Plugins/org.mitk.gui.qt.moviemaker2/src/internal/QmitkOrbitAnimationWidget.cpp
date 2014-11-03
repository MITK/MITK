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

#include "QmitkOrbitAnimationWidget.h"
#include <ui_QmitkOrbitAnimationWidget.h>

QmitkOrbitAnimationWidget::QmitkOrbitAnimationWidget(QWidget* parent)
  : QWidget(parent),
    m_Ui(new Ui::QmitkOrbitAnimationWidget)
{
  m_Ui->setupUi(this);
}

QmitkOrbitAnimationWidget::~QmitkOrbitAnimationWidget()
{
}
