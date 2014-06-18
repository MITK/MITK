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

#include <QmitkKinectParameterWidget.h>

const std::string QmitkKinectParameterWidget::VIEW_ID = "org.mitk.views.qmitkkinectparameterwidget";

QmitkKinectParameterWidget::QmitkKinectParameterWidget(QWidget* p, Qt::WindowFlags f): QWidget(p, f)
{
  m_ToFImageGrabber = NULL;
  m_Controls = NULL;
  CreateQtPartControl(this);
}

QmitkKinectParameterWidget::~QmitkKinectParameterWidget()
{
}

void QmitkKinectParameterWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkKinectParameterWidgetControls;
    m_Controls->setupUi(parent);

    this->CreateConnections();
  }
}

void QmitkKinectParameterWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->m_RGB, SIGNAL(toggled(bool)), this, SLOT(OnAcquisitionModeChanged()) );
    connect( m_Controls->m_IR, SIGNAL(toggled(bool)), this, SLOT(OnAcquisitionModeChanged()) );
    connect( m_Controls->m_RGB, SIGNAL(toggled(bool)), this, SIGNAL(AcquisitionModeChanged()) );
    connect( m_Controls->m_IR, SIGNAL(toggled(bool)), this, SIGNAL(AcquisitionModeChanged()) );
  }
}

mitk::ToFImageGrabber* QmitkKinectParameterWidget::GetToFImageGrabber()
{
  return this->m_ToFImageGrabber;
}

void QmitkKinectParameterWidget::SetToFImageGrabber(mitk::ToFImageGrabber* aToFImageGrabber)
{
  this->m_ToFImageGrabber = aToFImageGrabber;
}

void QmitkKinectParameterWidget::ActivateAllParameters()
{
  this->OnAcquisitionModeChanged();
}

void QmitkKinectParameterWidget::OnAcquisitionModeChanged()
{
  if (m_ToFImageGrabber.IsNotNull())
  {
    if (!m_ToFImageGrabber->IsCameraConnected())
    {
      this->m_ToFImageGrabber->SetBoolProperty("RGB", m_Controls->m_RGB->isChecked());
      this->m_ToFImageGrabber->SetBoolProperty("IR", m_Controls->m_IR->isChecked());
    }
    else
    {
      MITK_WARN << "Kinect Acquisition mode cannot be changed, if the device is connected. Please disconnect first.";
    }
  }
}

bool QmitkKinectParameterWidget::IsAcquisitionModeRGB()
{
  return m_Controls->m_RGB->isChecked();
}

bool QmitkKinectParameterWidget::IsAcquisitionModeIR()
{
  return m_Controls->m_IR->isChecked();
}

