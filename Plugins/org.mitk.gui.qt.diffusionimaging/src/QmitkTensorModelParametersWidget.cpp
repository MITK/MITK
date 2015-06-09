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
#include "QmitkTensorModelParametersWidget.h"
#include <itkDiffusionTensor3D.h>

const std::string QmitkTensorModelParametersWidget::VIEW_ID = "org.mitk.views.tensormodelparameterswidget";

QmitkTensorModelParametersWidget::QmitkTensorModelParametersWidget( QWidget * parent, Qt::WindowFlags )
    : QWidget(parent)
{
    m_Controls = nullptr;
    this->CreateQtPartControl(this);
}

QmitkTensorModelParametersWidget::~QmitkTensorModelParametersWidget()
{
}

void QmitkTensorModelParametersWidget::CreateQtPartControl(QWidget *parent)
{
    if (!m_Controls)
    {
        // create GUI widgets
        m_Controls = new Ui::QmitkTensorModelParametersWidgetControls;
        m_Controls->setupUi(parent);

        connect((QObject*) m_Controls->m_D1box, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(DChanged(double)));
        connect((QObject*) m_Controls->m_D2box, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(DChanged(double)));
        connect((QObject*) m_Controls->m_D3box, SIGNAL(valueChanged(double)), (QObject*) this, SLOT(DChanged(double)));

        UpdateUi();
    }
}

void QmitkTensorModelParametersWidget::UpdateUi()
{
    itk::DiffusionTensor3D<float> tensor;
    tensor.Fill(0);
    tensor.SetElement(0,m_Controls->m_D1box->value());
    tensor.SetElement(3,m_Controls->m_D2box->value());
    tensor.SetElement(5,m_Controls->m_D3box->value());
    m_Controls->m_FaLabel->setText(QString::number(tensor.GetFractionalAnisotropy()));
}

void QmitkTensorModelParametersWidget::DChanged( double )
{
    UpdateUi();
}
