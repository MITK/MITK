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

//Qmitk headers
#include "QmitkPrototypeSignalParametersWidget.h"
#include <itkDiffusionTensor3D.h>

const std::string QmitkPrototypeSignalParametersWidget::VIEW_ID = "org.mitk.views.prototypesignalparameterswidget";

QmitkPrototypeSignalParametersWidget::QmitkPrototypeSignalParametersWidget( QWidget * parent, Qt::WindowFlags )
    : QWidget(parent)
{
    m_Controls = NULL;
    this->CreateQtPartControl(this);
}

QmitkPrototypeSignalParametersWidget::~QmitkPrototypeSignalParametersWidget()
{
}

void QmitkPrototypeSignalParametersWidget::CreateQtPartControl(QWidget *parent)
{
    if (!m_Controls)
    {
        // create GUI widgets
        m_Controls = new Ui::QmitkPrototypeSignalParametersWidgetControls;
        m_Controls->setupUi(parent);

    }
}
