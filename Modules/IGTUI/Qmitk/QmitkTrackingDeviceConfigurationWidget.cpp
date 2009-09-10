/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: QmitkTrackingDeviceWidget.cpp $
Language:  C++
Date:      $Date: 2009-05-12 19:14:45 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkTrackingDeviceConfigurationWidget.h"
#include "mitkClaronTrackingDevice.h"
#include "mitkNDITrackingDevice.h"
#include "mitkSerialCommunication.h"

const std::string QmitkTrackingDeviceConfigurationWidget::VIEW_ID = "org.mitk.views.trackingdeviceconfigurationwidget";

QmitkTrackingDeviceConfigurationWidget::QmitkTrackingDeviceConfigurationWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_Controls = NULL;
  this->CreateQtPartControl(this);
  this->CreateConnections();
}


QmitkTrackingDeviceConfigurationWidget::~QmitkTrackingDeviceConfigurationWidget()
{
}

void QmitkTrackingDeviceConfigurationWidget::CreateQtPartControl(QWidget *parent)
{
  
  if (!m_Controls)
  {
  // create GUI widgets
  m_Controls = new Ui::QmitkTrackingDeviceConfigurationWidgetControls;
  m_Controls->setupUi(parent);
  }

  //create connections
  CreateConnections();
}

void QmitkTrackingDeviceConfigurationWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_trackingDeviceChooser), SIGNAL(currentIndexChanged(int)), this, SLOT(TrackingDeviceChanged()) );
  }
}

void QmitkTrackingDeviceConfigurationWidget::TrackingDeviceChanged()
{
  //show the correspondig widget
  m_Controls->m_TrackingSystemWidget->setCurrentIndex(m_Controls->m_trackingDeviceChooser->currentIndex());
  
  //the new trackingdevice is not configurated yet
  m_TrackingDeviceConfigurated = false;
  
  if (m_Controls->m_trackingDeviceChooser->currentIndex()==0)//NDI Polaris
      {
      m_Controls->m_outputTextPolaris->setHtml("<body bgcolor=black><span style=\"color:#ffffff;\"><u>output:</u><br> NDI Polaris selected");
      //m_Controls->m_outputTextPolaris->toPlainText().append(
      /* => später zu finished verschieben!
      mitk::NDITrackingDevice::Pointer tempTrackingDevice = mitk::NDITrackingDevice::New();
      tempTrackingDevice->SetType(mitk::NDIPolaris);
      
      switch (m_Controls->m_comPortSpinBoxPolaris->value())
        {
        case 1: tempTrackingDevice->SetPortNumber(mitk::SerialCommunication::COM1); break;
        case 2: tempTrackingDevice->SetPortNumber(mitk::SerialCommunication::COM2); break;
        case 3: tempTrackingDevice->SetPortNumber(mitk::SerialCommunication::COM3); break;
        case 4: tempTrackingDevice->SetPortNumber(mitk::SerialCommunication::COM4); break;
        case 5: tempTrackingDevice->SetPortNumber(mitk::SerialCommunication::COM5); break;
        case 6: tempTrackingDevice->SetPortNumber(mitk::SerialCommunication::COM6); break;
        case 7: tempTrackingDevice->SetPortNumber(mitk::SerialCommunication::COM7); break;
        case 8: tempTrackingDevice->SetPortNumber(mitk::SerialCommunication::COM8); break;
        }
      
      m_TrackingDevice = tempTrackingDevice;
      */
      } 
  else if (m_Controls->m_trackingDeviceChooser->currentIndex()==1)//NDI Aurora
      {
        m_Controls->m_outputTextAurora->setHtml("<body bgcolor=black><span style=\"color:#ffffff;\"><u>output:</u><br> NDI Aurora selected");
      }
  else if (m_Controls->m_trackingDeviceChooser->currentIndex()==2)//ClaronTechnology MicronTracker 2
      {
        m_Controls->m_outputTextMicronTracker->setHtml("<body bgcolor=black><span style=\"color:#ffffff;\"><u>output:</u><br> MicronTracker selected");
      }
}

void QmitkTrackingDeviceConfigurationWidget::EnableUserReset(bool enable)
{
  if (enable) m_Controls->m_resetButton->setVisible(true);
  else m_Controls->m_resetButton->setVisible(false);
}

void QmitkTrackingDeviceConfigurationWidget::TestConnection()
{
if (m_Controls->m_trackingDeviceChooser->currentIndex()==0)//NDI Polaris
      {
      }
else if (m_Controls->m_trackingDeviceChooser->currentIndex()==1)//NDI Aurora
      {
      }
else if (m_Controls->m_trackingDeviceChooser->currentIndex()==2)//ClaronTechnology MicronTracker 2
      {
      mitk::ClaronTrackingDevice::Pointer tempTrackingDevice = mitk::ClaronTrackingDevice::New();
      m_Controls->m_outputTextMicronTracker->setHtml(m_Controls->m_outputTextMicronTracker->toHtml().append("<br>Testing connect..."));
      }

}