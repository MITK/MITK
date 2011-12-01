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
#include <mitkClaronTrackingDevice.h>
#include <mitkNDITrackingDevice.h>
#include <mitkSerialCommunication.h>
#include <qscrollbar.h>
#include <qmessagebox.h>

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
  this->m_TrackingDeviceConfigurated = false;

  m_AdvancedUserControl = true;
}

void QmitkTrackingDeviceConfigurationWidget::SetGUIStyle(QmitkTrackingDeviceConfigurationWidget::Style style)
{
switch(style)
  {
  case QmitkTrackingDeviceConfigurationWidget::SIMPLE:

    //move all UI elements to an empty dummy layout
    //m_Controls->dummyLayout->addItem(m_Controls->mainLayout);
    m_Controls->dummyLayout->addWidget(m_Controls->widget_title_label);
    m_Controls->dummyLayout->addWidget(m_Controls->choose_tracking_device_label);
    m_Controls->dummyLayout->addWidget(m_Controls->polaris_label);
    m_Controls->dummyLayout->addWidget( m_Controls->aurora_label);
    m_Controls->dummyLayout->addWidget(m_Controls->aurora_label);
    m_Controls->dummyLayout->addWidget(m_Controls->microntracker_label);
    m_Controls->dummyLayout->addWidget(m_Controls->m_testConnectionMicronTracker);
    m_Controls->dummyLayout->addWidget(m_Controls->m_outputTextMicronTracker);
    m_Controls->dummyLayout->addWidget(m_Controls->m_outputTextAurora);
    m_Controls->dummyLayout->addWidget(m_Controls->m_testConnectionAurora);
    m_Controls->dummyLayout->addWidget(m_Controls->m_outputTextPolaris);
    m_Controls->dummyLayout->addWidget(m_Controls->m_testConnectionPolaris);
    m_Controls->dummyLayout->addWidget(m_Controls->m_polarisTrackingModeBox);
    m_Controls->dummyLayout->addWidget(m_Controls->m_finishedLine);
    m_Controls->dummyLayout->addWidget(m_Controls->line);
    m_Controls->dummyLayout->addWidget(m_Controls->configuration_finished_label);
    m_Controls->dummyLayout->addItem(m_Controls->horizontalLayout_4);
    m_Controls->mainLayout->removeItem(m_Controls->horizontalLayout_4);
    m_Controls->dummyLayout->addWidget(m_Controls->configuration_finished_label);  
    m_Controls->dummyLayout->addItem(m_Controls->verticalSpacer_2);
    m_Controls->verticalLayout_3->removeItem(m_Controls->verticalSpacer_2);
    m_Controls->dummyLayout->addItem(m_Controls->horizontalSpacer_9);
    m_Controls->horizontalLayout_9->removeItem(m_Controls->horizontalSpacer_9);
    m_Controls->dummyLayout->addItem(m_Controls->horizontalSpacer_3);
    m_Controls->horizontalLayout_11->removeItem(m_Controls->horizontalSpacer_3);
    m_Controls->dummyLayout->addItem(m_Controls->verticalSpacer_3);
    m_Controls->verticalLayout_7->removeItem(m_Controls->verticalSpacer_3);
    m_Controls->dummyLayout->addItem(m_Controls->verticalSpacer_4);
    m_Controls->verticalLayout_10->removeItem(m_Controls->verticalSpacer_4);
    m_Controls->dummyLayout->addItem(m_Controls->horizontalSpacer_10);
    m_Controls->verticalLayout_10->removeItem(m_Controls->horizontalSpacer_10);

    //set height to min
    m_Controls->m_outputTextPolaris->setMinimumHeight(0);
    m_Controls->m_outputTextPolaris->setMaximumHeight(0);
    m_Controls->m_outputTextMicronTracker->setMinimumHeight(0);
    m_Controls->m_outputTextMicronTracker->setMaximumHeight(0);
    m_Controls->m_outputTextAurora->setMinimumHeight(0);
    m_Controls->m_outputTextAurora->setMaximumHeight(0);
    m_Controls->m_finishedButton->setMinimumHeight(0);
    m_Controls->m_finishedButton->setMaximumHeight(0);
    m_Controls->m_resetButton->setMinimumHeight(0);
    m_Controls->m_resetButton->setMaximumHeight(0);

 
    //set the height of the tracking device combo box
    m_Controls->m_trackingDeviceChooser->setMinimumHeight(50);

    //move back the used elemets to the main layout
    m_Controls->simpleLayout->addWidget(m_Controls->m_trackingDeviceChooser);
    m_Controls->simpleLayout->addWidget(m_Controls->m_TrackingSystemWidget);

    m_Controls->mainWidget->setCurrentIndex(1);

    this->setMaximumHeight(150);

    this->EnableAdvancedUserControl(false);
    

    break;

  case QmitkTrackingDeviceConfigurationWidget::ADVANCED:

    //default at the moment => start settings are advanced

    break;

  }

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
    connect( (QObject*)(m_Controls->m_AutoScanPolaris), SIGNAL(clicked()), this, SLOT(AutoScanPorts()) );
    connect( (QObject*)(m_Controls->m_AutoScanAurora), SIGNAL(clicked()), this, SLOT(AutoScanPorts()) );

    //set a few UI components depending on Windows / Linux
    #ifdef WIN32
    m_Controls->portTypeLabelPolaris->setVisible(false);
    m_Controls->portTypePolaris->setVisible(false);
    m_Controls->portTypeLabelAurora->setVisible(false);
    m_Controls->portTypeAurora->setVisible(false);
    #else
    m_Controls->comPortLabelAurora->setText("Port Nr:");
    m_Controls->m_comPortLabelPolaris->setText("Port Nr:");
    m_Controls->m_portSpinBoxAurora->setPrefix("");
    m_Controls->m_portSpinBoxPolaris->setPrefix("");
    #endif

    //disable unused UI component
    m_Controls->m_polarisTrackingModeBox->setVisible(false); //don't delete this component, because it is used in the MBI part of MITK
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

  emit TrackingDeviceSelectionChanged();
}

void QmitkTrackingDeviceConfigurationWidget::EnableUserReset(bool enable)
{
  if (enable) m_Controls->m_resetButton->setVisible(true);
  else m_Controls->m_resetButton->setVisible(false);
}

void QmitkTrackingDeviceConfigurationWidget::TestConnection()
{
this->setEnabled(false);

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

this->setEnabled(true);
}

void QmitkTrackingDeviceConfigurationWidget::Finished()
  {
  m_TrackingDevice = ConstructTrackingDevice();
  m_Controls->m_TrackingSystemWidget->setEnabled(false);
  m_Controls->m_trackingDeviceChooser->setEnabled(false);
  m_Controls->choose_tracking_device_label->setEnabled(false);
  m_Controls->configuration_finished_label->setText("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\np, li { white-space: pre-wrap; }\n</style></head><body style=\" font-family:\'MS Shell Dlg 2\'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\"><span style=\" font-weight:600;\">Configuration finished</span></p></body></html>");
  this->m_TrackingDeviceConfigurated = true;
  emit TrackingDeviceConfigurationFinished();
  }

void QmitkTrackingDeviceConfigurationWidget::Reset()
  {
  m_TrackingDevice = NULL;
  m_Controls->m_TrackingSystemWidget->setEnabled(true);
  m_Controls->m_trackingDeviceChooser->setEnabled(true);
  m_Controls->choose_tracking_device_label->setEnabled(true);
  m_Controls->configuration_finished_label->setText("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\np, li { white-space: pre-wrap; }\n</style></head><body style=\" font-family:\'MS Shell Dlg 2\'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n<p align=\"right\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\"><span style=\" font-weight:600;\">Press \"Finished\" to confirm configuration</span></p></body></html>");
  this->m_TrackingDeviceConfigurated = false;
  emit TrackingDeviceConfigurationReseted();
  }

void QmitkTrackingDeviceConfigurationWidget::ResetByUser()
  {
  Reset();
  }

void QmitkTrackingDeviceConfigurationWidget::AutoScanPorts()
  {
  this->setEnabled(false);
  AddOutput("<br>Scanning...");

  QString result = "<br>Found Devices:";
  int resultSize = result.size(); //remember size of result: if it stays the same no device were found

  #ifdef WIN32
    QString devName;
    for (unsigned int i = 1; i < 20; ++i)
    {
      if (i<10) devName = QString("COM%1").arg(i);
      else devName = QString("\\\\.\\COM%1").arg(i); // prepend "\\.\ to COM ports >9, to be able to allow connection"
      mitk::TrackingDeviceType scannedPort = ScanPort(devName);
      switch (scannedPort)
      {
      case mitk::NDIPolaris:
        result += "<br>" + devName + ": " + "NDI Polaris";
        m_Controls->m_portSpinBoxPolaris->setValue(i);
        break;
      case mitk::NDIAurora:
        result += "<br>" + devName + ": " + "NDI Aurora";
        m_Controls->m_portSpinBoxAurora->setValue(i);
        break;
      }
    }
  #else //linux systems
    for(unsigned int i = 1; i < 6; ++i)
    {
      QString devName = QString("/dev/ttyS%1").arg(i);
      mitk::TrackingDeviceType scannedPort = ScanPort(devName);
      switch (scannedPort)
      {
      case mitk::NDIPolaris:
        result += "<br>" + devName + ": " + "NDI Polaris";
        m_Controls->m_portSpinBoxPolaris->setValue(i);
        m_Controls->portTypePolaris->setCurrentIndex(1);
        break;
      case mitk::NDIAurora:
        result += "<br>" + devName + ": " + "NDI Aurora";
        m_Controls->m_portSpinBoxAurora->setValue(i);
        m_Controls->portTypeAurora->setCurrentIndex(1);
        break;
      }
      
    }
    for(unsigned int i = 0; i <7; ++i)
    {
      QString devName = QString("/dev/ttyUSB%1").arg(i);
      mitk::TrackingDeviceType scannedPort = ScanPort(devName);
      switch (scannedPort)
      {
      case mitk::NDIPolaris:
        result += "<br>" + devName + ": " + "NDI Polaris";
        m_Controls->m_portSpinBoxPolaris->setValue(i);
        m_Controls->portTypePolaris->setCurrentIndex(0);
        break;
      case mitk::NDIAurora:
        result += "<br>" + devName + ": " + "NDI Aurora";
        m_Controls->m_portSpinBoxAurora->setValue(i);
        m_Controls->portTypeAurora->setCurrentIndex(0);
        break;
      }
      
    }
  #endif
      
  if ( result.size() == resultSize) result += "<br>none";

  AddOutput(result.toStdString());

  this->setEnabled(true);
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
  
  //get port
  int port = 0;
  if (m_Controls->m_trackingDeviceChooser->currentIndex()==1) port = m_Controls->m_portSpinBoxAurora->value();
  else port = m_Controls->m_portSpinBoxPolaris->value();

  //build prefix (depends on linux/win)
  QString prefix = "";
  #ifdef WIN32
  prefix ="COM";
  tempTrackingDevice->SetPortNumber(static_cast<mitk::SerialCommunication::PortNumber>(port)); //also set the com port for compatibility
  #else
  if (m_Controls->m_trackingDeviceChooser->currentIndex()==1) //Aurora
    prefix = m_Controls->portTypeAurora->currentText();
  else //Polaris
    prefix = m_Controls->portTypePolaris->currentText();
  #endif
  
  //build port name string
  QString portName = prefix + QString::number(port);

  tempTrackingDevice->SetDeviceName(portName.toStdString()); //set the port name
  mitk::TrackingDevice::Pointer returnValue = static_cast<mitk::TrackingDevice*>(tempTrackingDevice);
  return returnValue;
  }

mitk::TrackingDevice::Pointer QmitkTrackingDeviceConfigurationWidget::GetTrackingDevice()
  {
  if (!m_AdvancedUserControl) m_TrackingDevice = ConstructTrackingDevice();
  return this->m_TrackingDevice;
  }

bool QmitkTrackingDeviceConfigurationWidget::GetTrackingDeviceConfigured()
  {
  return this->m_TrackingDeviceConfigurated;
  }

void QmitkTrackingDeviceConfigurationWidget::ConfigurationFinished()
  {
  Finished();

  }

void QmitkTrackingDeviceConfigurationWidget::EnableAdvancedUserControl(bool enable)
  {
  m_AdvancedUserControl = enable;
  m_Controls->configuration_finished_label->setVisible(enable);
  m_Controls->m_finishedLine->setVisible(enable);
  m_Controls->m_resetButton->setVisible(enable);
  m_Controls->m_finishedButton->setVisible(enable);
  }

mitk::TrackingDeviceType QmitkTrackingDeviceConfigurationWidget::ScanPort(QString port)
{
  mitk::NDITrackingDevice::Pointer tracker = mitk::NDITrackingDevice::New();
  tracker->SetDeviceName(port.toStdString());
  return tracker->TestConnection();
}

