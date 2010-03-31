/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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
#include "qscrollbar.h"

const std::string QmitkTrackingDeviceConfigurationWidget::VIEW_ID = "org.mitk.views.trackingdeviceconfigurationwidget";

QmitkTrackingDeviceConfigurationWidget::QmitkTrackingDeviceConfigurationWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
  CreateConnections();
  
  //reset a few things
  ResetOutput();
  AddOutput("<br>NDI Polaris selected");
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
}

void QmitkTrackingDeviceConfigurationWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_trackingDeviceChooser), SIGNAL(currentIndexChanged(int)), this, SLOT(TrackingDeviceChanged()) );
    connect( (QObject*)(m_Controls->m_testConnectionPolaris), SIGNAL(clicked()), this, SLOT(TestConnection()) );
    connect( (QObject*)(m_Controls->m_testConnectionAurora), SIGNAL(clicked()), this, SLOT(TestConnection()) );
    connect( (QObject*)(m_Controls->m_testConnectionMicronTracker), SIGNAL(clicked()), this, SLOT(TestConnection()) );
    connect( (QObject*)(m_Controls->m_resetButton), SIGNAL(clicked()), this, SLOT(ResetByUser()) );
    connect( (QObject*)(m_Controls->m_finishedButton), SIGNAL(clicked()), this, SLOT(Finished()) );
  }
}

void QmitkTrackingDeviceConfigurationWidget::TrackingDeviceChanged()
{
  //show the correspondig widget
  m_Controls->m_TrackingSystemWidget->setCurrentIndex(m_Controls->m_trackingDeviceChooser->currentIndex());
  
  //the new trackingdevice is not configurated yet
  m_TrackingDeviceConfigurated = false;
  
  //reset output
  ResetOutput();

  //print output
  if (m_Controls->m_trackingDeviceChooser->currentIndex()==0) AddOutput("<br>NDI Polaris selected");        //NDI Polaris
  else if (m_Controls->m_trackingDeviceChooser->currentIndex()==1) AddOutput("<br>NDI Aurora selected");    //NDI Aurora
  else if (m_Controls->m_trackingDeviceChooser->currentIndex()==2) AddOutput("<br>Microntracker selected"); //ClaronTechnology MicronTracker 2
}

void QmitkTrackingDeviceConfigurationWidget::EnableUserReset(bool enable)
{
  if (enable) m_Controls->m_resetButton->setVisible(true);
  else m_Controls->m_resetButton->setVisible(false);
}

void QmitkTrackingDeviceConfigurationWidget::TestConnection()
{
//#### Step 1: construct a tracking device:
mitk::TrackingDevice::Pointer testTrackingDevice = ConstructTrackingDevice();

//#### Step 2: test connection and start tracking, generate output
AddOutput("<br>testing connection <br>  ...");
if (testTrackingDevice->OpenConnection())
  {
  AddOutput(" OK");
  AddOutput("<br>testing tracking <br>  ...");
  if (testTrackingDevice->StartTracking()) 
    {
    AddOutput(" OK");
    if (!testTrackingDevice->StopTracking())AddOutput("<br>ERROR while stop tracking<br>");
    }
  else AddOutput(" ERROR!");
  if (!testTrackingDevice->CloseConnection())AddOutput("<br>ERROR while closing connection<br>");
  }
else AddOutput(" ERROR!");
}

void QmitkTrackingDeviceConfigurationWidget::Finished()
  {
  m_TrackingDevice = ConstructTrackingDevice();
  m_Controls->m_TrackingSystemWidget->setEnabled(false);
  m_Controls->m_trackingDeviceChooser->setEnabled(false);
  m_Controls->choose_tracking_device_label->setEnabled(false);
  m_Controls->configuration_finished_label->setText("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\np, li { white-space: pre-wrap; }\n</style></head><body style=\" font-family:\'MS Shell Dlg 2\'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\"><span style=\" font-weight:600;\">Configuration finished</span></p></body></html>");
  emit TrackingDeviceConfigurationFinished();
  }

void QmitkTrackingDeviceConfigurationWidget::Reset()
  {
  m_TrackingDevice = NULL;
  m_Controls->m_TrackingSystemWidget->setEnabled(true);
  m_Controls->m_trackingDeviceChooser->setEnabled(true);
  m_Controls->choose_tracking_device_label->setEnabled(true);
  m_Controls->configuration_finished_label->setText("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\np, li { white-space: pre-wrap; }\n</style></head><body style=\" font-family:\'MS Shell Dlg 2\'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\"><span style=\" font-weight:600;\">Press \"Finished\" to confirm configuration</span></p></body></html>");
  emit TrackingDeviceConfigurationReseted();
  }

void QmitkTrackingDeviceConfigurationWidget::ResetByUser()
  {
  Reset();
  }

//######################### internal help methods #######################################
void QmitkTrackingDeviceConfigurationWidget::ResetOutput()
  {
  m_output.str("");
  m_output <<"<body style=\" font-family:\'MS Shell Dlg 2\'; font-size:7pt; font-weight:400; font-style:normal;\" bgcolor=black><span style=\"color:#ffffff;\"><u>output:</u>";
  m_Controls->m_outputTextAurora->setHtml(QString(m_output.str().c_str()));
  m_Controls->m_outputTextPolaris->setHtml(QString(m_output.str().c_str()));
  m_Controls->m_outputTextMicronTracker->setHtml(QString(m_output.str().c_str()));
  }
void QmitkTrackingDeviceConfigurationWidget::AddOutput(std::string s)
  {
  //print output
  m_output << s;
  m_Controls->m_outputTextAurora->setHtml(QString(m_output.str().c_str()));
  m_Controls->m_outputTextPolaris->setHtml(QString(m_output.str().c_str()));
  m_Controls->m_outputTextMicronTracker->setHtml(QString(m_output.str().c_str()));
  m_Controls->m_outputTextPolaris->verticalScrollBar()->setValue(m_Controls->m_outputTextPolaris->verticalScrollBar()->maximum());
  m_Controls->m_outputTextAurora->verticalScrollBar()->setValue(m_Controls->m_outputTextAurora->verticalScrollBar()->maximum());
  m_Controls->m_outputTextMicronTracker->verticalScrollBar()->setValue(m_Controls->m_outputTextMicronTracker->verticalScrollBar()->maximum());
  repaint();
  }
mitk::TrackingDevice::Pointer QmitkTrackingDeviceConfigurationWidget::ConstructTrackingDevice()
  {
  mitk::TrackingDevice::Pointer returnValue;
  //#### Step 1: configure tracking device:
  if (m_Controls->m_trackingDeviceChooser->currentIndex()==0)//NDI Polaris
      {
      if(m_Controls->m_radioPolaris5D->isChecked()) //5D Tracking
        {
        //not yet in the open source part so we'll only get NULL here.
        returnValue = ConfigureNDI5DTrackingDevice();
        }
      else //6D Tracking
        {
        returnValue = ConfigureNDI6DTrackingDevice();
        returnValue->SetType(mitk::NDIPolaris);
        }
      }
  else if (m_Controls->m_trackingDeviceChooser->currentIndex()==1)//NDI Aurora
        {
        returnValue = ConfigureNDI6DTrackingDevice();
        returnValue->SetType(mitk::NDIAurora);
        }
  else if (m_Controls->m_trackingDeviceChooser->currentIndex()==2)//ClaronTechnology MicronTracker 2
        {
        returnValue = mitk::ClaronTrackingDevice::New();
        }
  return returnValue;
  }

mitk::TrackingDevice::Pointer QmitkTrackingDeviceConfigurationWidget::ConfigureNDI5DTrackingDevice()
  {
  return NULL;
  }

mitk::TrackingDevice::Pointer QmitkTrackingDeviceConfigurationWidget::ConfigureNDI6DTrackingDevice()
  {
  mitk::NDITrackingDevice::Pointer tempTrackingDevice = mitk::NDITrackingDevice::New();
  int comPort = 0;
  if (m_Controls->m_trackingDeviceChooser->currentIndex()==1) comPort = m_Controls->m_comPortSpinBoxAurora->value();
  else comPort = m_Controls->m_comPortSpinBoxPolaris->value();
  tempTrackingDevice->SetPortNumber(static_cast<mitk::SerialCommunication::PortNumber>(comPort)); //set the com port
  mitk::TrackingDevice::Pointer returnValue = static_cast<mitk::TrackingDevice*>(tempTrackingDevice);
  return returnValue;
  }

mitk::TrackingDevice::Pointer QmitkTrackingDeviceConfigurationWidget::GetTrackingDevice()
  {
  return this->m_TrackingDevice;
  }

