/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-20 13:35:09 +0200 (Mi, 20 Mai 2009) $
Version:   $Revision: 17332 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
    // stop camera if active
    bool active = m_ToFImageGrabber->IsCameraActive();
    if (active)
    {
      m_ToFImageGrabber->StopCamera();
      m_ToFImageGrabber->DisconnectCamera();
    }
    this->m_ToFImageGrabber->SetBoolProperty("RGB", m_Controls->m_RGB->isChecked());
    this->m_ToFImageGrabber->SetBoolProperty("IR", m_Controls->m_IR->isChecked());
    if (active)
    {
      m_ToFImageGrabber->ConnectCamera();
      m_ToFImageGrabber->StartCamera();
    }
  }
}

