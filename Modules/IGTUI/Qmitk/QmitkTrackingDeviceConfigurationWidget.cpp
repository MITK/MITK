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

#include "QmitkTrackingDeviceConfigurationWidget.h"
#include <mitkClaronTrackingDevice.h>
#include <mitkNDITrackingDevice.h>
#include <mitkOptitrackTrackingDevice.h>
#include <mitkIGTException.h>
#include <mitkSerialCommunication.h>
#include <mitkProgressBar.h>
#include <qscrollbar.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <mitkIGTException.h>
#include <QSettings>

#include <itksys/SystemTools.hxx>
#include <Poco/Path.h>



const std::string QmitkTrackingDeviceConfigurationWidget::VIEW_ID = "org.mitk.views.trackingdeviceconfigurationwidget";

QmitkTrackingDeviceConfigurationWidget::QmitkTrackingDeviceConfigurationWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  //initialize worker thread
  m_TestConnectionWorker = new QmitkTrackingDeviceConfigurationWidgetConnectionWorker();
  m_ScanPortsWorker = new QmitkTrackingDeviceConfigurationWidgetScanPortsWorker();
  m_ScanPortsWorkerThread = new QThread();
  m_TestConnectionWorkerThread = new QThread();

   //initializations
  m_Controls = NULL;
  CreateQtPartControl(this);
  CreateConnections();
  m_MTCalibrationFile = "";
  m_AdvancedUserControl = true;

  //initialize a few UI elements
  this->m_TrackingDeviceConfigurated = false;
  AddOutput("<br>NDI Polaris selected"); //Polaris is selected by default
  m_Controls->m_trackingDeviceChooser->setCurrentIndex(0);
  m_Controls->m_TrackingSystemWidget->setCurrentIndex(0);
  //reset a few things
  ResetOutput();

  //restore old UI settings
  LoadUISettings();
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
    m_Controls->dummyLayout->addWidget(m_Controls->aurora_label);
    //m_Controls->dummyLayout->addWidget(m_Controls->aurora_label);
    m_Controls->dummyLayout->addWidget(m_Controls->microntracker_label);
    m_Controls->dummyLayout->addWidget(m_Controls->m_testConnectionMicronTracker);
    m_Controls->dummyLayout->addWidget(m_Controls->m_outputTextMicronTracker);
    m_Controls->dummyLayout->addWidget(m_Controls->m_outputTextAurora);
    m_Controls->dummyLayout->addWidget(m_Controls->m_testConnectionAurora);
    m_Controls->dummyLayout->addWidget(m_Controls->m_outputTextPolaris);
    m_Controls->dummyLayout->addWidget(m_Controls->m_testConnectionPolaris);
    m_Controls->dummyLayout->addWidget(m_Controls->m_polarisTrackingModeBox);
    m_Controls->dummyLayout->addWidget(m_Controls->m_testConnectionOptitrack);
    m_Controls->dummyLayout->addWidget(m_Controls->m_outputTextOptitrack);
    m_Controls->dummyLayout->addWidget(m_Controls->m_OptitrackExp);
    m_Controls->dummyLayout->addWidget(m_Controls->m_OptitrackThr);
    m_Controls->dummyLayout->addWidget(m_Controls->m_OptitrackLed);
    m_Controls->dummyLayout->addWidget(m_Controls->Optitrack_label);
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
StoreUISettings();
if (m_ScanPortsWorker) delete m_ScanPortsWorker;
if (m_TestConnectionWorker) delete m_TestConnectionWorker;
if (m_ScanPortsWorkerThread) delete m_ScanPortsWorkerThread;
if (m_TestConnectionWorkerThread) delete m_TestConnectionWorkerThread;
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
    connect( (QObject*)(m_Controls->m_testConnectionOptitrack), SIGNAL(clicked()), this, SLOT(TestConnection()) );
    connect( (QObject*)(m_Controls->m_resetButton), SIGNAL(clicked()), this, SLOT(ResetByUser()) );
    connect( (QObject*)(m_Controls->m_finishedButton), SIGNAL(clicked()), this, SLOT(Finished()) );
    connect( (QObject*)(m_Controls->m_AutoScanPolaris), SIGNAL(clicked()), this, SLOT(AutoScanPorts()) );
    connect( (QObject*)(m_Controls->m_AutoScanAurora), SIGNAL(clicked()), this, SLOT(AutoScanPorts()) );
    connect( (QObject*)(m_Controls->m_SetMTCalibrationFile), SIGNAL(clicked()), this, SLOT(SetMTCalibrationFileClicked()) );
    connect( (QObject*)(m_Controls->m_SetOptitrackCalibrationFile), SIGNAL(clicked()), this, SLOT(SetOptitrackCalibrationFileClicked()) );

    //slots for the worker thread
    connect(m_ScanPortsWorker, SIGNAL(PortsScanned(int,int,QString,int,int)), this, SLOT(AutoScanPortsFinished(int,int,QString,int,int)) );
    connect(m_TestConnectionWorker, SIGNAL(ConnectionTested(bool,QString)), this, SLOT(TestConnectionFinished(bool,QString)) );
    connect(m_ScanPortsWorkerThread,SIGNAL(started()), m_ScanPortsWorker, SLOT(ScanPortsThreadFunc()) );
    connect(m_TestConnectionWorkerThread,SIGNAL(started()), m_TestConnectionWorker, SLOT(TestConnectionThreadFunc()) );

    //move the worker to the thread
    m_ScanPortsWorker->moveToThread(m_ScanPortsWorkerThread);
    m_TestConnectionWorker->moveToThread(m_TestConnectionWorkerThread);

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

  //print output and do further initializations
  if (m_Controls->m_trackingDeviceChooser->currentIndex()==0)//NDI Polaris
    {
    AddOutput("<br>NDI Polaris selected");
    }
  else if (m_Controls->m_trackingDeviceChooser->currentIndex()==1)  //NDI Aurora
    {
    AddOutput("<br>NDI Aurora selected");
    }
  else if (m_Controls->m_trackingDeviceChooser->currentIndex()==2) //ClaronTechnology MicronTracker 2
    {
    AddOutput("<br>Microntracker selected");
    if (!mitk::ClaronTrackingDevice::New()->IsDeviceInstalled())
      {
      AddOutput("<br>ERROR: not installed!");
      }
    else if (this->m_MTCalibrationFile == "") //if configuration file for MicronTracker is empty: load default
      {
      mitk::ClaronTrackingDevice::Pointer tempDevice = mitk::ClaronTrackingDevice::New();
      m_MTCalibrationFile = tempDevice->GetCalibrationDir();
      Poco::Path myPath = Poco::Path(m_MTCalibrationFile.c_str());
      m_Controls->m_MTCalibrationFile->setText("Calibration File: " + QString(myPath.getFileName().c_str()));
      }
    }
  else if (m_Controls->m_trackingDeviceChooser->currentIndex()==3)
  {
    AddOutput("<br>Optitrack selected");
    if (!mitk::OptitrackTrackingDevice::New()->IsDeviceInstalled())
      {
      AddOutput("<br>ERROR: not installed!");
      }
  }

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
//construct a tracking device:
mitk::TrackingDevice::Pointer testTrackingDevice = ConstructTrackingDevice();
m_TestConnectionWorker->SetTrackingDevice(testTrackingDevice);
m_TestConnectionWorkerThread->start();
emit ProgressStarted();
}

void QmitkTrackingDeviceConfigurationWidget::TestConnectionFinished(bool connected, QString output)
{
m_TestConnectionWorkerThread->quit();
AddOutput(output.toStdString());
MITK_INFO << "Test connection: " << connected;
this->setEnabled(true);
emit ProgressFinished();
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
  m_ScanPortsWorkerThread->start();
  emit ProgressStarted();
  }

void QmitkTrackingDeviceConfigurationWidget::AutoScanPortsFinished(int PolarisPort, int AuroraPort, QString result, int PortTypePolaris, int PortTypeAurora)
  {
  m_ScanPortsWorkerThread->quit();
  #ifdef WIN32
    if((PortTypePolaris!=-1)||(PortTypeAurora!=-1)) {MITK_WARN << "Port type is specified although this should not be the case for Windows. Ignoring port type.";}
  #else //linux systems
    if (PortTypePolaris!=-1) {m_Controls->portTypePolaris->setCurrentIndex(PortTypePolaris);}
    if (PortTypeAurora!=-1)  {m_Controls->portTypeAurora->setCurrentIndex(PortTypeAurora);}
  #endif
  m_Controls->m_portSpinBoxPolaris->setValue(PolarisPort);
  m_Controls->m_portSpinBoxAurora->setValue(AuroraPort);
  AddOutput(result.toStdString());
  this->setEnabled(true);
  emit ProgressFinished();
  }

void QmitkTrackingDeviceConfigurationWidget::SetMTCalibrationFileClicked()
  {
  std::string filename = QFileDialog::getOpenFileName(NULL,tr("Open Calibration File"), "/", "*.*").toLatin1().data();
  if (filename=="") {return;}
  else
    {
    m_MTCalibrationFile = filename;
    Poco::Path myPath = Poco::Path(m_MTCalibrationFile.c_str());
    m_Controls->m_MTCalibrationFile->setText("Calibration File: " + QString(myPath.getFileName().c_str()));
    }
  }

void QmitkTrackingDeviceConfigurationWidget::SetOptitrackCalibrationFileClicked()
  {
  std::string filename = QFileDialog::getOpenFileName(NULL,tr("Open Calibration File"), "/", "*.*").toLatin1().data();
  if (filename=="") {return;}
  else
    {
    m_OptitrackCalibrationFile = filename;
    Poco::Path myPath = Poco::Path(m_OptitrackCalibrationFile.c_str());
    m_Controls->m_OptitrackCalibrationFile->setText("Calibration File: " + QString(myPath.getFileName().c_str()));
    }
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
  m_Controls->m_outputTextOptitrack->setHtml(QString(m_output.str().c_str()));
  m_Controls->m_outputTextPolaris->verticalScrollBar()->setValue(m_Controls->m_outputTextPolaris->verticalScrollBar()->maximum());
  m_Controls->m_outputTextAurora->verticalScrollBar()->setValue(m_Controls->m_outputTextAurora->verticalScrollBar()->maximum());
  m_Controls->m_outputTextMicronTracker->verticalScrollBar()->setValue(m_Controls->m_outputTextMicronTracker->verticalScrollBar()->maximum());
  m_Controls->m_outputTextOptitrack->verticalScrollBar()->setValue(m_Controls->m_outputTextOptitrack->verticalScrollBar()->maximum());
  repaint();
  }
mitk::TrackingDevice::Pointer QmitkTrackingDeviceConfigurationWidget::ConstructTrackingDevice()
  {
  mitk::TrackingDevice::Pointer returnValue;
  //#### Step 1: configure tracking device:
  MITK_INFO << "Current Index: " << m_Controls->m_trackingDeviceChooser->currentIndex();
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

        mitk::ClaronTrackingDevice::Pointer newDevice = mitk::ClaronTrackingDevice::New();
        if(this->m_MTCalibrationFile=="") AddOutput("<br>Warning: Calibration file is not set!");
        else
          {
          //extract path from calibration file and set the calibration dir of the device
          std::string path =  itksys::SystemTools::GetFilenamePath(m_MTCalibrationFile);
          newDevice->SetCalibrationDir(path);
          }
        returnValue = newDevice;
        }
  else if (m_Controls->m_trackingDeviceChooser->currentIndex()==3)
  {
    // Create the Tracking Device this->m_OptitrackDevice = mitk::OptitrackTrackingDevice::New();
    returnValue = ConfigureOptitrackTrackingDevice();
    returnValue->SetType(mitk::NPOptitrack);
  }
  return returnValue;
  }

mitk::TrackingDevice::Pointer QmitkTrackingDeviceConfigurationWidget::ConfigureNDI5DTrackingDevice()
  {
  return NULL;
  }

mitk::TrackingDevice::Pointer QmitkTrackingDeviceConfigurationWidget::ConfigureOptitrackTrackingDevice()
  {
  mitk::OptitrackTrackingDevice::Pointer tempTrackingDevice = mitk::OptitrackTrackingDevice::New();
  // Set the calibration File
  tempTrackingDevice->SetCalibrationPath(m_OptitrackCalibrationFile);

  //Set the camera parameters
  tempTrackingDevice->SetExp(m_Controls->m_OptitrackExp->value());
  tempTrackingDevice->SetLed(m_Controls->m_OptitrackLed->value());
  tempTrackingDevice->SetThr(m_Controls->m_OptitrackThr->value());

  mitk::TrackingDevice::Pointer returnValue = static_cast<mitk::TrackingDevice*>(tempTrackingDevice);
  return returnValue;
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
  tempTrackingDevice->SetBaudRate(mitk::SerialCommunication::BaudRate115200);//set baud rate
  mitk::TrackingDevice::Pointer returnValue = static_cast<mitk::TrackingDevice*>(tempTrackingDevice);
  return returnValue;
  }

mitk::TrackingDevice::Pointer QmitkTrackingDeviceConfigurationWidget::GetTrackingDevice()
  {
  if (!m_AdvancedUserControl) m_TrackingDevice = ConstructTrackingDevice();
  if (m_TrackingDevice.IsNull() || !m_TrackingDevice->IsDeviceInstalled()) return NULL;
  else return this->m_TrackingDevice;
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


void QmitkTrackingDeviceConfigurationWidget::StoreUISettings()
{
  std::string id = "org.mitk.modules.igt.ui.trackingdeviceconfigurationwidget";
  int selectedDevice = m_Controls->m_trackingDeviceChooser->currentIndex();

  if ( this->GetPeristenceService() ) // now save the settings using the persistence service
  {
    mitk::PropertyList::Pointer propList = this->GetPeristenceService()->GetPropertyList(id);
    propList->Set("PolarisPortWin",m_Controls->m_portSpinBoxPolaris->value());
    propList->Set("AuroraPortWin",m_Controls->m_portSpinBoxAurora->value());
    propList->Set("PortTypePolaris", m_Controls->portTypePolaris->currentIndex());
    propList->Set("PortTypeAurora", m_Controls->portTypeAurora->currentIndex());
    propList->Set("MTCalibrationFile",m_MTCalibrationFile);
    propList->Set("SelectedDevice",selectedDevice);
  }
  else // QSettings as a fallback if the persistence service is not available
  {
    QSettings settings;
    settings.beginGroup(QString::fromStdString(id));
    settings.setValue("trackingDeviceChooser", QVariant(selectedDevice));
    settings.setValue("portSpinBoxAurora", QVariant(m_Controls->m_portSpinBoxAurora->value()));
    settings.setValue("portSpinBoxPolaris", QVariant(m_Controls->m_portSpinBoxPolaris->value()));
    settings.setValue("portTypePolaris", QVariant(m_Controls->portTypePolaris->currentIndex()));
    settings.setValue("portTypeAurora", QVariant(m_Controls->portTypeAurora->currentIndex()));
    settings.setValue("mTCalibrationFile", QVariant(QString::fromStdString(m_MTCalibrationFile)));
    settings.endGroup();
  }
}

void QmitkTrackingDeviceConfigurationWidget::LoadUISettings()
{
  std::string id = "org.mitk.modules.igt.ui.trackingdeviceconfigurationwidget";
  int SelectedDevice = 0;
  if ( this->GetPeristenceService() )
  {
    mitk::PropertyList::Pointer propList = this->GetPeristenceService()->GetPropertyList(id);
    if (propList.IsNull())
    {MITK_ERROR << "Property list for this UI (" << id <<") is not available, could not load UI settings!"; return;}
    int portPolarisWin,portAuroraWin,portTypePolaris,portTypeAurora;
    propList->Get("PolarisPortWin",portPolarisWin);
    propList->Get("AuroraPortWin",portAuroraWin);
    propList->Get("PortTypePolaris", portTypePolaris);
    propList->Get("PortTypeAurora", portTypeAurora);
    propList->Get("MTCalibrationFile",m_MTCalibrationFile);
    propList->Get("SelectedDevice",SelectedDevice);

    if (SelectedDevice<0)
    {
      MITK_ERROR << "Loaded data from persistence service is invalid (SelectedDevice:" <<SelectedDevice<<"): aborted to restore data!";
      return;
    }
    m_Controls->m_portSpinBoxPolaris->setValue(portPolarisWin);
    m_Controls->m_portSpinBoxAurora->setValue(portAuroraWin);
    m_Controls->portTypePolaris->setCurrentIndex(portTypePolaris);
    m_Controls->portTypeAurora->setCurrentIndex(portTypeAurora);
    MITK_INFO << "Sucessfully restored UI settings";
  }
  else
  {
    // QSettings as a fallback if the persistence service is not available
    QSettings settings;
    settings.beginGroup(QString::fromStdString(id));

    SelectedDevice = settings.value("trackingDeviceChooser", 0).toInt();
    m_Controls->m_portSpinBoxAurora->setValue(settings.value("portSpinBoxAurora", 0).toInt());
    m_Controls->m_portSpinBoxPolaris->setValue(settings.value("portSpinBoxPolaris", 0).toInt());
    m_Controls->portTypePolaris->setCurrentIndex(settings.value("portTypePolaris", 0).toInt());
    m_Controls->portTypeAurora->setCurrentIndex(settings.value("portTypeAurora", 0).toInt());
    m_MTCalibrationFile = settings.value("mTCalibrationFile", "").toString().toStdString();

    settings.endGroup();
  }

  //the selected device requires some checks because a device that is not installed should not be restored to avoids bugs
  int selectedDeviceChecked = SelectedDevice;
  if (SelectedDevice==2 && !mitk::ClaronTrackingDevice::New()->IsDeviceInstalled())
    {selectedDeviceChecked = 0;} //0 = Polaris (default)
  else if (SelectedDevice==3 && !mitk::OptitrackTrackingDevice::New()->IsDeviceInstalled())
    {selectedDeviceChecked = 0;}
  MITK_INFO << "SelectedDeviceChecked: " << selectedDeviceChecked;
  m_Controls->m_TrackingSystemWidget->setCurrentIndex(selectedDeviceChecked);
  m_Controls->m_trackingDeviceChooser->setCurrentIndex(selectedDeviceChecked);

  m_Controls->m_MTCalibrationFile->setText("Calibration File: " + QString::fromStdString(m_MTCalibrationFile));
}
void QmitkTrackingDeviceConfigurationWidgetConnectionWorker::TestConnectionThreadFunc()
{
MITK_INFO << "Testing Connection!";
QString output;
bool connected = false;
mitk::ProgressBar::GetInstance()->AddStepsToDo(4);
try
  {
  if (!m_TrackingDevice->IsDeviceInstalled())
     {
     output = "ERROR: Device is not installed!";
     }
  else
    {
    //test connection and start tracking, generate output
    output = "<br>testing connection <br>  ...";
    m_TrackingDevice->OpenConnection();
    output += "OK";
    mitk::ProgressBar::GetInstance()->Progress();

    //try start/stop tracking
    output += "<br>testing tracking <br>  ...";
    m_TrackingDevice->StartTracking();
    mitk::ProgressBar::GetInstance()->Progress();
    m_TrackingDevice->StopTracking();
    mitk::ProgressBar::GetInstance()->Progress();

    //try close connection
    m_TrackingDevice->CloseConnection();
    mitk::ProgressBar::GetInstance()->Progress();
    output += "OK";
    connected = true;
    }
  }
catch(mitk::IGTException &e)
  {
  output += "ERROR!";
  MITK_WARN << "Error while testing connection / start tracking of the device: " << e.GetDescription();
  }
mitk::ProgressBar::GetInstance()->Progress(4);
emit ConnectionTested(connected,output);
}

void QmitkTrackingDeviceConfigurationWidgetScanPortsWorker::ScanPortsThreadFunc()
{
  int PolarisPort = -1;
  int AuroraPort = -1;
  int PortTypePolaris = -1;
  int PortTypeAurora = -1;

  QString result = "<br>Found Devices:";
  int resultSize = result.size(); //remember size of result: if it stays the same no device were found

  #ifdef WIN32
    mitk::ProgressBar::GetInstance()->AddStepsToDo(19);

    QString devName;
    for (unsigned int i = 1; i < 20; ++i)
    {
      QString statusOutput = "Scanning Port #" + QString::number(i);
      MITK_INFO << statusOutput.toStdString().c_str();
      if (i<10) devName = QString("COM%1").arg(i);
      else devName = QString("\\\\.\\COM%1").arg(i); // prepend "\\.\ to COM ports >9, to be able to allow connection"
      mitk::TrackingDeviceType scannedPort = ScanPort(devName);
      switch (scannedPort)
      {
      case mitk::NDIPolaris:
        result += "<br>" + devName + ": " + "NDI Polaris";
        PolarisPort = i;
        break;
      case mitk::NDIAurora:
        result += "<br>" + devName + ": " + "NDI Aurora";
        AuroraPort = i;
        break;
      }
      mitk::ProgressBar::GetInstance()->Progress();
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
        PolarisPort = i;
        PortTypePolaris = 1;
        break;
      case mitk::NDIAurora:
        result += "<br>" + devName + ": " + "NDI Aurora";
        AuroraPort = i;
        PortTypeAurora = 1;
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
        PolarisPort = i;
        PortTypePolaris = 0;
        break;
      case mitk::NDIAurora:
        result += "<br>" + devName + ": " + "NDI Aurora";
        AuroraPort = i;
        PortTypeAurora = 0;
        break;
      }

    }
  #endif

  if ( result.size() == resultSize) result += "<br>none";

  emit PortsScanned(PolarisPort,AuroraPort,result,PortTypePolaris,PortTypeAurora);
}

mitk::TrackingDeviceType QmitkTrackingDeviceConfigurationWidgetScanPortsWorker::ScanPort(QString port)
{
  mitk::NDITrackingDevice::Pointer tracker = mitk::NDITrackingDevice::New();
  tracker->SetDeviceName(port.toStdString());
  mitk::TrackingDeviceType returnValue = mitk::TrackingSystemInvalid;
  try
  {returnValue = tracker->TestConnection();}
  catch (mitk::IGTException)
  {}//do nothing: there is simply no device on this port
  return returnValue;
}

void QmitkTrackingDeviceConfigurationWidgetConnectionWorker::SetTrackingDevice(mitk::TrackingDevice::Pointer t)
{
  m_TrackingDevice = t;
}
