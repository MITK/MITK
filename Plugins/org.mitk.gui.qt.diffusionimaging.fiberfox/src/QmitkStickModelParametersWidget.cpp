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
#include "QmitkStickModelParametersWidget.h"
#include <itkDiffusionTensor3D.h>

const std::string QmitkStickModelParametersWidget::VIEW_ID = "org.mitk.views.StickModelParametersWidget";

QmitkStickModelParametersWidget::QmitkStickModelParametersWidget( QWidget * parent, Qt::WindowFlags )
    : QWidget(parent)
{
    m_Controls = nullptr;
    this->CreateQtPartControl(this);
}

QmitkStickModelParametersWidget::~QmitkStickModelParametersWidget()
{
}

void QmitkStickModelParametersWidget::CreateQtPartControl(QWidget *parent)
{
    if (!m_Controls)
    {
        // create GUI widgets
        m_Controls = new Ui::QmitkStickModelParametersWidgetControls;
        m_Controls->setupUi(parent);
    }
}
