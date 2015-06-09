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

#include <QmitkStructureSensorParameterWidget.h>

const std::string QmitkStructureSensorParameterWidget::VIEW_ID = "org.mitk.views.qmitkStructureSensorparameterwidget";

QmitkStructureSensorParameterWidget::QmitkStructureSensorParameterWidget(QWidget* p, Qt::WindowFlags f): QWidget(p, f)
{
  m_ToFImageGrabber = NULL;
  m_Controls = NULL;
  CreateQtPartControl(this);
}

QmitkStructureSensorParameterWidget::~QmitkStructureSensorParameterWidget()
{
}

void QmitkStructureSensorParameterWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkStructureSensorParameterWidgetControls;
    m_Controls->setupUi(parent);

    this->CreateConnections();
  }
}

void QmitkStructureSensorParameterWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->m_640, SIGNAL(toggled(bool)), this, SLOT(OnResolutionChanged()) );
    connect( m_Controls->m_320, SIGNAL(toggled(bool)), this, SLOT(OnResolutionChanged()) );
  }
}

mitk::ToFImageGrabber* QmitkStructureSensorParameterWidget::GetToFImageGrabber()
{
  return this->m_ToFImageGrabber;
}

void QmitkStructureSensorParameterWidget::SetToFImageGrabber(mitk::ToFImageGrabber* aToFImageGrabber)
{
  this->m_ToFImageGrabber = aToFImageGrabber;
}

void QmitkStructureSensorParameterWidget::ActivateAllParameters()
{
  this->OnResolutionChanged();
}

void QmitkStructureSensorParameterWidget::OnResolutionChanged()
{
  if (m_ToFImageGrabber.IsNotNull())
  {
    if (!m_ToFImageGrabber->IsCameraConnected())
    {
      //for know depth and RGB resolution is the same, but this could be different in future
      this->m_ToFImageGrabber->SetIntProperty("RGBResolution", GetSelectedResolution());
      this->m_ToFImageGrabber->SetIntProperty("DepthResolution", GetSelectedResolution());
    }
    else
    {
      MITK_WARN << "Structure Sensor resolution cannot be changed, if the device is connected. Please disconnect first.";
    }
  }
}

int QmitkStructureSensorParameterWidget::GetSelectedResolution()
{
  if(m_Controls->m_640->isChecked())
  {
    return 640;
  }
  else if(m_Controls->m_320->isChecked())
  {
    return 320;
  }
  else
  {
    MITK_ERROR << "Unsupported structure sensor resolution!";
    return -1;
  }
}
