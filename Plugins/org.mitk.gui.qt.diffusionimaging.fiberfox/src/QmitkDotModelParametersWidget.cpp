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
#include "QmitkDotModelParametersWidget.h"
#include <itkDiffusionTensor3D.h>

const std::string QmitkDotModelParametersWidget::VIEW_ID = "org.mitk.views.DotModelParametersWidget";

QmitkDotModelParametersWidget::QmitkDotModelParametersWidget( QWidget * parent, Qt::WindowFlags )
    : QWidget(parent)
{
    m_Controls = nullptr;
    this->CreateQtPartControl(this);
}

QmitkDotModelParametersWidget::~QmitkDotModelParametersWidget()
{
}

void QmitkDotModelParametersWidget::CreateQtPartControl(QWidget *parent)
{
    if (!m_Controls)
    {
        // create GUI widgets
        m_Controls = new Ui::QmitkDotModelParametersWidgetControls;
        m_Controls->setupUi(parent);
    }
}
