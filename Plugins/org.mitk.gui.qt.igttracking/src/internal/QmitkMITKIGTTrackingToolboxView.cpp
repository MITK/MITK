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

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkMITKIGTTrackingToolboxView.h"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>
#include <QSettings>
#include <qfiledialog.h>

// MITK
#include <mitkNavigationToolStorageDeserializer.h>
#include <mitkTrackingDeviceSourceConfigurator.h>
#include <mitkTrackingVolumeGenerator.h>
#include <mitkNDITrackingDevice.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNavigationToolStorageSerializer.h>
#include <mitkProgressBar.h>
#include <mitkNavigationDataSetWriterXML.h>
#include <mitkNavigationDataSetWriterCSV.h>

// vtk
#include <vtkSphereSource.h>
//for exceptions
#include <mitkIGTException.h>
#include <mitkIGTIOException.h>

const std::string QmitkMITKIGTTrackingToolboxView::VIEW_ID = "org.mitk.views.mitkigttrackingtoolbox";

QmitkMITKIGTTrackingToolboxView::QmitkMITKIGTTrackingToolboxView()
  : QmitkFunctionality()
  , m_Controls( 0 )
  , m_MultiWidget( NULL )
{
  m_TrackingTimer = new QTimer(this);
  m_TimeoutTimer = new QTimer(this);
  m_tracking = false;
  m_connected = false;
  m_logging = false;
  m_loggedFrames = 0;

  //initialize worker thread
  m_WorkerThread = new QThread();
  m_Worker = new QmitkMITKIGTTrackingToolboxViewWorker();
}

QmitkMITKIGTTrackingToolboxView::~QmitkMITKIGTTrackingToolboxView()
{
this->StoreUISettings();
m_TrackingTimer->stop();
m_TimeoutTimer->stop();
delete m_TrackingTimer;
delete m_TimeoutTimer;
try
  {
  // wait for thread to finish
  m_WorkerThread->terminate();
  m_WorkerThread->wait();
  //clean up worker thread
  if(m_WorkerThread) {delete m_WorkerThread;}
  if(m_Worker) {delete m_Worker;}
  //remove the tracking volume
  this->GetDataStorage()->Remove(m_TrackingVolumeNode);
  //remove the tool storage
  if(m_toolStorage) {m_toolStorage->UnRegisterMicroservice();}
  if(m_TrackingDeviceSource) {m_TrackingDeviceSource->UnRegisterMicroservice();}
  }
  catch(std::exception& e) {MITK_WARN << "Unexpected exception during clean up of tracking toolbox view: " << e.what();}
  catch(...) {MITK_WARN << "Unexpected unknown error during clean up of tracking toolbox view!";}
  this->StoreUISettings();
}

void QmitkMITKIGTTrackingToolboxView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkMITKIGTTrackingToolboxViewControls;
    m_Controls->setupUi( parent );

    //create connections
    connect( m_Controls->m_LoadTools, SIGNAL(clicked()), this, SLOT(OnLoadTools()) );
    connect( m_Controls->m_ConnectDisconnectButton, SIGNAL(clicked()), this, SLOT(OnConnectDisconnect()) );
    connect( m_Controls->m_StartStopTrackingButton, SIGNAL(clicked()), this, SLOT(OnStartStopTracking()) );
    connect( m_TrackingTimer, SIGNAL(timeout()), this, SLOT(UpdateTrackingTimer()));
    connect( m_TimeoutTimer, SIGNAL(timeout()), this, SLOT(OnTimeOut()));
    connect( m_Controls->m_ChooseFile, SIGNAL(clicked()), this, SLOT(OnChooseFileClicked()));
    connect( m_Controls->m_StartLogging, SIGNAL(clicked()), this, SLOT(StartLogging()));
    connect( m_Controls->m_StopLogging, SIGNAL(clicked()), this, SLOT(StopLogging()));
    connect( m_Controls->m_VolumeSelectionBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(OnTrackingVolumeChanged(QString)));
    connect( m_Controls->m_ShowTrackingVolume, SIGNAL(clicked()), this, SLOT(OnShowTrackingVolumeChanged()));
    connect( m_Controls->m_AutoDetectTools, SIGNAL(clicked()), this, SLOT(OnAutoDetectTools()));
    connect( m_Controls->m_ResetTools, SIGNAL(clicked()), this, SLOT(OnResetTools()));
    connect( m_Controls->m_AddSingleTool, SIGNAL(clicked()), this, SLOT(OnAddSingleTool()));
    connect( m_Controls->m_NavigationToolCreationWidget, SIGNAL(NavigationToolFinished()), this, SLOT(OnAddSingleToolFinished()));
    connect( m_Controls->m_NavigationToolCreationWidget, SIGNAL(Canceled()), this, SLOT(OnAddSingleToolCanceled()));
    connect( m_Controls->m_csvFormat, SIGNAL(clicked()), this, SLOT(OnToggleFileExtension()));
    connect( m_Controls->m_xmlFormat, SIGNAL(clicked()), this, SLOT(OnToggleFileExtension()));

    //connections for the tracking device configuration widget
    connect( m_Controls->m_configurationWidget, SIGNAL(TrackingDeviceSelectionChanged()), this, SLOT(OnTrackingDeviceChanged()));
    connect( m_Controls->m_configurationWidget, SIGNAL(ProgressStarted()), this, SLOT(DisableOptionsButtons()));
    connect( m_Controls->m_configurationWidget, SIGNAL(ProgressStarted()), this, SLOT(DisableTrackingConfigurationButtons()));
    connect( m_Controls->m_configurationWidget, SIGNAL(ProgressStarted()), this, SLOT(DisableTrackingControls()));
    connect( m_Controls->m_configurationWidget, SIGNAL(ProgressFinished()), this, SLOT(EnableOptionsButtons()));
    connect( m_Controls->m_configurationWidget, SIGNAL(ProgressFinished()), this, SLOT(EnableTrackingConfigurationButtons()));
    connect( m_Controls->m_configurationWidget, SIGNAL(ProgressFinished()), this, SLOT(EnableTrackingControls()));

    //connect worker thread
    connect(m_Worker, SIGNAL(AutoDetectToolsFinished(bool,QString)), this, SLOT(OnAutoDetectToolsFinished(bool,QString)) );
    connect(m_Worker, SIGNAL(ConnectDeviceFinished(bool,QString)), this, SLOT(OnConnectFinished(bool,QString)) );
    connect(m_Worker, SIGNAL(StartTrackingFinished(bool,QString)), this, SLOT(OnStartTrackingFinished(bool,QString)) );
    connect(m_Worker, SIGNAL(StopTrackingFinished(bool,QString)), this, SLOT(OnStopTrackingFinished(bool,QString)) );
    connect(m_Worker, SIGNAL(DisconnectDeviceFinished(bool,QString)), this, SLOT(OnDisconnectFinished(bool,QString)) );
    connect(m_WorkerThread,SIGNAL(started()), m_Worker, SLOT(ThreadFunc()) );

    //move the worker to the thread
    m_Worker->moveToThread(m_WorkerThread);

    //initialize widgets
    m_Controls->m_configurationWidget->EnableAdvancedUserControl(false);
    m_Controls->m_TrackingToolsStatusWidget->SetShowPositions(true);
    m_Controls->m_TrackingToolsStatusWidget->SetTextAlignment(Qt::AlignLeft);

    //initialize tracking volume node
    m_TrackingVolumeNode = mitk::DataNode::New();
    m_TrackingVolumeNode->SetName("TrackingVolume");
    m_TrackingVolumeNode->SetBoolProperty("Backface Culling",true);
    mitk::Color red;
    red.SetRed(1);
    m_TrackingVolumeNode->SetColor(red);

    //initialize buttons
    m_Controls->m_AutoDetectTools->setVisible(false); //only visible if tracking device is Aurora
    m_Controls->m_StartStopTrackingButton->setEnabled(false);

    //Update List of available models for selected tool.
    std::vector<mitk::TrackingDeviceData> Compatibles;
    if ( (m_Controls == NULL) || //check all these stuff for NULL, latterly this causes crashes from time to time
      (m_Controls->m_configurationWidget == NULL) ||
      (m_Controls->m_configurationWidget->GetTrackingDevice().IsNull()))
    {
      MITK_ERROR << "Couldn't get current tracking device or an object is NULL, something went wrong!";
      return;
    }
    else
    {
      Compatibles = mitk::GetDeviceDataForLine( m_Controls->m_configurationWidget->GetTrackingDevice()->GetType());
    }
    m_Controls->m_VolumeSelectionBox->clear();
    for(int i = 0; i < Compatibles.size(); i++)
    {
      m_Controls->m_VolumeSelectionBox->addItem(Compatibles[i].Model.c_str());
    }

    //initialize tool storage
    m_toolStorage = mitk::NavigationToolStorage::New(GetDataStorage());
    m_toolStorage->SetName("TrackingToolbox Default Storage");
    m_toolStorage->RegisterAsMicroservice("no tracking device");

    //set home directory as default path for logfile
    m_Controls->m_LoggingFileName->setText(QDir::toNativeSeparators(QDir::homePath()) + QDir::separator() + "logfile.csv");

    //tracking device may be changed already by the persistence of the
    //QmitkTrackingDeciveConfigurationWidget
    this->OnTrackingDeviceChanged();

    this->LoadUISettings();

    //add tracking volume node only to data storage
    this->GetDataStorage()->Add(m_TrackingVolumeNode);
    if (!m_Controls->m_ShowTrackingVolume->isChecked()) m_TrackingVolumeNode->SetOpacity(0.0);
    else m_TrackingVolumeNode->SetOpacity(0.25);

    //Update List of available models for selected tool.
    m_Controls->m_VolumeSelectionBox->clear();
    for(int i = 0; i < Compatibles.size(); i++)
    {
      m_Controls->m_VolumeSelectionBox->addItem(Compatibles[i].Model.c_str());
    }
  }
}

void QmitkMITKIGTTrackingToolboxView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}

void QmitkMITKIGTTrackingToolboxView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkMITKIGTTrackingToolboxView::OnLoadTools()
{
  //read in filename
  QString filename = QFileDialog::getOpenFileName(NULL,tr("Open Tool Storage"), "/", tr("Tool Storage Files (*.IGTToolStorage)"));
  if (filename.isNull()) return;

  //read tool storage from disk
  std::string errorMessage = "";
  mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(GetDataStorage());
  // try-catch block for exceptions
  try
  {
    this->ReplaceCurrentToolStorage(myDeserializer->Deserialize(filename.toStdString()),filename.toStdString());
  }
  catch(mitk::IGTException)
  {
   std::string errormessage = "Error during loading the tool storage file. Please only load tool storage files created with the NavigationToolManager view.";
   QMessageBox::warning(NULL, "Tool Storage Loading Error", errormessage.c_str());
   return;
  }

  if(m_toolStorage->isEmpty())
  {
    errorMessage = myDeserializer->GetErrorMessage();
    MessageBox(errorMessage);
    return;
  }

  //update label
  UpdateToolStorageLabel(filename);

  //update tool preview
  m_Controls->m_TrackingToolsStatusWidget->RemoveStatusLabels();
  m_Controls->m_TrackingToolsStatusWidget->PreShowTools(m_toolStorage);

  //save filename for persistent storage
  m_ToolStorageFilename = filename;
}

void QmitkMITKIGTTrackingToolboxView::OnResetTools()
{
  this->ReplaceCurrentToolStorage(mitk::NavigationToolStorage::New(GetDataStorage()),"TrackingToolbox Default Storage");
  m_Controls->m_TrackingToolsStatusWidget->RemoveStatusLabels();
  QString toolLabel = QString("Loaded Tools: <none>");
  m_Controls->m_toolLabel->setText(toolLabel);

  m_ToolStorageFilename = "";
}

void QmitkMITKIGTTrackingToolboxView::OnStartStopTracking()
  {
  if(!m_connected)
    {
    MITK_WARN << "Can't start tracking if no device is connected. Aborting";
    return;
    }
  if(m_tracking) {OnStopTracking();}
  else {OnStartTracking();}
  }

void QmitkMITKIGTTrackingToolboxView::OnConnectDisconnect()
  {
  if(m_connected) {OnDisconnect();}
  else {OnConnect();}
  }

void QmitkMITKIGTTrackingToolboxView::OnConnect()
{
  MITK_INFO << "Connect Clicked";
  //check if everything is ready to start tracking
  if (this->m_toolStorage.IsNull())
  {
    MessageBox("Error: No Tools Loaded Yet!");
    return;
  }
  else if (this->m_toolStorage->GetToolCount() == 0)
  {
    MessageBox("Error: No Way To Track Without Tools!");
    return;
  }

  //parse tracking device data
  mitk::TrackingDeviceData data = mitk::DeviceDataUnspecified;
  QString qstr =  m_Controls->m_VolumeSelectionBox->currentText();
  if ( (! qstr.isNull()) || (! qstr.isEmpty()) ) {
    std::string str = qstr.toStdString();
    data = mitk::GetDeviceDataByName(str); //Data will be set later, after device generation
  }

  //initialize worker thread
  m_Worker->SetWorkerMethod(QmitkMITKIGTTrackingToolboxViewWorker::eConnectDevice);
  m_Worker->SetTrackingDevice(this->m_Controls->m_configurationWidget->GetTrackingDevice());
  m_Worker->SetInverseMode(m_Controls->m_InverseMode->isChecked());
  m_Worker->SetNavigationToolStorage(this->m_toolStorage);
  m_Worker->SetTrackingDeviceData(data);

  //start worker thread
  m_WorkerThread->start();

  //disable buttons
  this->m_Controls->m_MainWidget->setEnabled(false);
}

void QmitkMITKIGTTrackingToolboxView::OnConnectFinished(bool success, QString errorMessage)
{
  m_WorkerThread->quit();
  m_WorkerThread->wait();

  //enable buttons
  this->m_Controls->m_MainWidget->setEnabled(true);

  if (!success)
  {
    MITK_WARN << errorMessage.toStdString();
    MessageBox(errorMessage.toStdString());
    return;
  }

  //get data from worker thread
  m_TrackingDeviceSource = m_Worker->GetTrackingDeviceSource();
  m_TrackingDeviceData = m_Worker->GetTrackingDeviceData();
  m_ToolVisualizationFilter = m_Worker->GetToolVisualizationFilter();

  //enable/disable Buttons
  DisableOptionsButtons();
  DisableTrackingConfigurationButtons();
  m_Controls->m_configurationWidget->ConfigurationFinished();

  m_Controls->m_TrackingControlLabel->setText("Status: connected");
  m_Controls->m_ConnectDisconnectButton->setText("Disconnect");
  m_Controls->m_StartStopTrackingButton->setEnabled(true);
  m_connected = true;
  }

void QmitkMITKIGTTrackingToolboxView::OnDisconnect()
{
  m_Worker->SetWorkerMethod(QmitkMITKIGTTrackingToolboxViewWorker::eDisconnectDevice);
  m_WorkerThread->start();
  m_Controls->m_MainWidget->setEnabled(false);
}

void QmitkMITKIGTTrackingToolboxView::OnDisconnectFinished(bool success, QString errorMessage)
{
  m_WorkerThread->quit();
  m_WorkerThread->wait();
  m_Controls->m_MainWidget->setEnabled(true);

  if (!success)
  {
    MITK_WARN << errorMessage.toStdString();
    MessageBox(errorMessage.toStdString());
    return;
  }

  //enable/disable Buttons
  m_Controls->m_StartStopTrackingButton->setEnabled(false);
  EnableOptionsButtons();
  EnableTrackingConfigurationButtons();
  m_Controls->m_configurationWidget->Reset();
  m_Controls->m_TrackingControlLabel->setText("Status: disconnected");
  m_Controls->m_ConnectDisconnectButton->setText("Connect");

  m_connected = false;
  }

void QmitkMITKIGTTrackingToolboxView::OnStartTracking()
{
  m_Worker->SetWorkerMethod(QmitkMITKIGTTrackingToolboxViewWorker::eStartTracking);
  m_WorkerThread->start();
  this->m_Controls->m_MainWidget->setEnabled(false);
}

void QmitkMITKIGTTrackingToolboxView::OnStartTrackingFinished(bool success, QString errorMessage)
{
  m_WorkerThread->quit();
  m_WorkerThread->wait();
  this->m_Controls->m_MainWidget->setEnabled(true);

  if(!success)
  {
    MessageBox(errorMessage.toStdString());
    MITK_WARN << errorMessage.toStdString();
    return;
  }

  m_TrackingTimer->start(1000/(m_Controls->m_UpdateRate->value()));
  m_Controls->m_TrackingControlLabel->setText("Status: tracking");

  //connect the tool visualization widget
  for(int i=0; i<m_TrackingDeviceSource->GetNumberOfOutputs(); i++)
  {
    m_Controls->m_TrackingToolsStatusWidget->AddNavigationData(m_TrackingDeviceSource->GetOutput(i));
  }
  m_Controls->m_TrackingToolsStatusWidget->ShowStatusLabels();
  if (m_Controls->m_ShowToolQuaternions->isChecked()) {m_Controls->m_TrackingToolsStatusWidget->SetShowQuaternions(true);}
  else {m_Controls->m_TrackingToolsStatusWidget->SetShowQuaternions(false);}

  //show tracking volume
  this->OnTrackingVolumeChanged(m_Controls->m_VolumeSelectionBox->currentText());

  m_tracking = true;
  m_Controls->m_ConnectDisconnectButton->setEnabled(false);
  m_Controls->m_StartStopTrackingButton->setText("Stop Tracking");

  this->GlobalReinit();
}

void QmitkMITKIGTTrackingToolboxView::OnStopTracking()
{
  if (!m_tracking) return;
  m_TrackingTimer->stop();

  m_Worker->SetWorkerMethod(QmitkMITKIGTTrackingToolboxViewWorker::eStopTracking);
  m_WorkerThread->start();
  m_Controls->m_MainWidget->setEnabled(false);
}

void QmitkMITKIGTTrackingToolboxView::OnStopTrackingFinished(bool success, QString errorMessage)
{
  m_WorkerThread->quit();
  m_WorkerThread->wait();
  m_Controls->m_MainWidget->setEnabled(true);
  if(!success)
  {
    MessageBox(errorMessage.toStdString());
    MITK_WARN << errorMessage.toStdString();
    return;
  }

  m_Controls->m_TrackingControlLabel->setText("Status: connected");
  if (m_logging) StopLogging();
  m_Controls->m_TrackingToolsStatusWidget->RemoveStatusLabels();
  m_Controls->m_TrackingToolsStatusWidget->PreShowTools(m_toolStorage);
  m_tracking = false;
  m_Controls->m_StartStopTrackingButton->setText("Start Tracking");
  m_Controls->m_ConnectDisconnectButton->setEnabled(true);

  this->GlobalReinit();
}

void QmitkMITKIGTTrackingToolboxView::OnTrackingDeviceChanged()
{
  mitk::TrackingDeviceType Type;

  if (m_Controls->m_configurationWidget->GetTrackingDevice().IsNotNull())
  {
    Type = m_Controls->m_configurationWidget->GetTrackingDevice()->GetType();
    //enable controls because device is valid
    m_Controls->m_TrackingToolsGoupBox->setEnabled(true);
    m_Controls->m_TrackingControlsGroupBox->setEnabled(true);
  }
  else
  {
    Type = mitk::TrackingSystemNotSpecified;
    MessageBox("Error: This tracking device is not included in this project. Please make sure that the device is installed and activated in your MITK build.");
    m_Controls->m_TrackingToolsGoupBox->setEnabled(false);
    m_Controls->m_TrackingControlsGroupBox->setEnabled(false);
    return;
  }

  // Code to enable/disable device specific buttons
  if (Type == mitk::NDIAurora) //Aurora
  {
    m_Controls->m_AutoDetectTools->setVisible(true);
    m_Controls->m_AddSingleTool->setEnabled(false);
  }
  else //Polaris or Microntracker
  {
    m_Controls->m_AutoDetectTools->setVisible(false);
    m_Controls->m_AddSingleTool->setEnabled(true);
  }

  // Code to select appropriate tracking volume for current type
  std::vector<mitk::TrackingDeviceData> Compatibles = mitk::GetDeviceDataForLine(Type);
  m_Controls->m_VolumeSelectionBox->clear();
  for(int i = 0; i < Compatibles.size(); i++)
  {
    m_Controls->m_VolumeSelectionBox->addItem(Compatibles[i].Model.c_str());
  }
}

void QmitkMITKIGTTrackingToolboxView::OnTrackingVolumeChanged(QString qstr)
{
  if (qstr.isNull()) return;
  if (qstr.isEmpty()) return;

  mitk::TrackingVolumeGenerator::Pointer volumeGenerator = mitk::TrackingVolumeGenerator::New();

  std::string str = qstr.toStdString();

  mitk::TrackingDeviceData data = mitk::GetDeviceDataByName(str);
  m_TrackingDeviceData = data;

  volumeGenerator->SetTrackingDeviceData(data);
  volumeGenerator->Update();

  mitk::Surface::Pointer volumeSurface = volumeGenerator->GetOutput();

  m_TrackingVolumeNode->SetData(volumeSurface);

  if (!m_Controls->m_ShowTrackingVolume->isChecked()) m_TrackingVolumeNode->SetOpacity(0.0);
  else m_TrackingVolumeNode->SetOpacity(0.25);

  GlobalReinit();
}

void QmitkMITKIGTTrackingToolboxView::OnShowTrackingVolumeChanged()
{
  if (m_Controls->m_ShowTrackingVolume->isChecked())
  {
    OnTrackingVolumeChanged(m_Controls->m_VolumeSelectionBox->currentText());
    m_TrackingVolumeNode->SetOpacity(0.25);
  }
  else
  {
    m_TrackingVolumeNode->SetOpacity(0.0);
  }
}

void QmitkMITKIGTTrackingToolboxView::OnAutoDetectTools()
{
  if (m_Controls->m_configurationWidget->GetTrackingDevice()->GetType() == mitk::NDIAurora)
  {
    DisableTrackingConfigurationButtons();
    m_Worker->SetWorkerMethod(QmitkMITKIGTTrackingToolboxViewWorker::eAutoDetectTools);
    m_Worker->SetTrackingDevice(m_Controls->m_configurationWidget->GetTrackingDevice().GetPointer());
    m_Worker->SetDataStorage(this->GetDataStorage());
    m_WorkerThread->start();
    m_TimeoutTimer->start(5000);
    MITK_INFO << "Timeout Timer started";
    //disable controls until worker thread is finished
    this->m_Controls->m_MainWidget->setEnabled(false);
  }
}

void QmitkMITKIGTTrackingToolboxView::OnAutoDetectToolsFinished(bool success, QString errorMessage)
{
    m_TimeoutTimer->stop();
    m_WorkerThread->quit();
    m_WorkerThread->wait();

  //enable controls again
  this->m_Controls->m_MainWidget->setEnabled(true);

  if(!success)
  {
    MITK_WARN << errorMessage.toStdString();
    MessageBox(errorMessage.toStdString());
    EnableTrackingConfigurationButtons();
    return;
  }

  mitk::NavigationToolStorage::Pointer autoDetectedStorage = m_Worker->GetNavigationToolStorage();

  //save detected tools
  this->ReplaceCurrentToolStorage(autoDetectedStorage,"Autodetected NDI Aurora Storage");
  //update label
  QString toolLabel = QString("Loaded Tools: ") + QString::number(m_toolStorage->GetToolCount()) + " Tools (Auto Detected)";
  m_Controls->m_toolLabel->setText(toolLabel);
  //update tool preview
  m_Controls->m_TrackingToolsStatusWidget->RemoveStatusLabels();
  m_Controls->m_TrackingToolsStatusWidget->PreShowTools(m_toolStorage);

  EnableTrackingConfigurationButtons();

  if (m_toolStorage->GetToolCount()>0)
  {
    //ask the user if he wants to save the detected tools
    QMessageBox msgBox;
    switch(m_toolStorage->GetToolCount())
    {
    case 1:
      msgBox.setText("Found one tool!");
      break;
    default:
      msgBox.setText("Found " + QString::number(m_toolStorage->GetToolCount()) + " tools!");
    }
    msgBox.setInformativeText("Do you want to save this tools as tool storage, so you can load them again?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if (ret == 16384) //yes
    {
      //ask the user for a filename
      QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save File"),"/",tr("*.IGTToolStorage"));
      //check for empty filename
      if(fileName == "") {return;}
      mitk::NavigationToolStorageSerializer::Pointer mySerializer = mitk::NavigationToolStorageSerializer::New();

      //when Serialize method is used exceptions are thrown, need to be adapted
      //try-catch block for exception handling in Serializer
      try
      {
        mySerializer->Serialize(fileName.toStdString(),m_toolStorage);
      }
      catch(mitk::IGTException)
      {
        std::string errormessage = "Error during serialization. Please check the Zip file.";
        QMessageBox::warning(NULL, "IGTPlayer: Error", errormessage.c_str());
      }

      return;
    }
    else if (ret == 65536) //no
    {
      return;
    }
  }
}

void QmitkMITKIGTTrackingToolboxView::MessageBox(std::string s)
{
  QMessageBox msgBox;
  msgBox.setText(s.c_str());
  msgBox.exec();
}

void QmitkMITKIGTTrackingToolboxView::UpdateTrackingTimer()
{
  //update filter
  m_ToolVisualizationFilter->Update();
  MITK_DEBUG << "Number of outputs ToolVisualizationFilter: " << m_ToolVisualizationFilter->GetNumberOfIndexedOutputs();
  MITK_DEBUG << "Number of inputs ToolVisualizationFilter: " << m_ToolVisualizationFilter->GetNumberOfIndexedInputs();

  //update tool colors to show tool status
  for(int i=0; i<m_ToolVisualizationFilter->GetNumberOfIndexedOutputs(); i++)
    {
    mitk::NavigationData::Pointer currentTool = m_ToolVisualizationFilter->GetOutput(i);
    if(currentTool->IsDataValid())
      {this->m_toolStorage->GetTool(i)->GetDataNode()->SetColor(mitk::IGTColor_VALID);}
    else
      {this->m_toolStorage->GetTool(i)->GetDataNode()->SetColor(mitk::IGTColor_WARNING);}
    }

  //update logging
  if (m_logging)
  {
    this->m_loggingFilter->Update();
    m_loggedFrames = this->m_loggingFilter->GetNumberOfRecordedSteps();
    this->m_Controls->m_LoggedFramesLabel->setText("Logged Frames: "+QString::number(m_loggedFrames));
    //check if logging stopped automatically
    if((m_loggedFrames>1)&&(!m_loggingFilter->GetRecording())){StopLogging();}
  }

  //refresh view and status widget
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  m_Controls->m_TrackingToolsStatusWidget->Refresh();

  //code to better isolate bug 17713, could be removed when bug 17713 is fixed
  static int i = 0;
  static mitk::Point3D lastPositionTool1 = m_ToolVisualizationFilter->GetOutput(0)->GetPosition();
  static itk::TimeStamp lastTimeStamp = m_ToolVisualizationFilter->GetOutput(0)->GetTimeStamp();
  i++;
  //every 20 frames: check if tracking is frozen
  if(i>20)
    {
    i = 0;
    if (m_ToolVisualizationFilter->GetOutput(0)->IsDataValid())
      {
      if (mitk::Equal(lastPositionTool1,m_ToolVisualizationFilter->GetOutput(0)->GetPosition(),0.000000001,false))
        {
        MITK_WARN << "Seems as tracking (of at least tool 1) is frozen which means that bug 17713 occurred. Restart tracking might help.";
        //display further information to find the bug
        MITK_WARN << "Timestamp of current navigation data: " << m_ToolVisualizationFilter->GetOutput(0)->GetTimeStamp();
        MITK_WARN << "Timestamp of last navigation data (which holds the same values): " << lastTimeStamp;
        }
      lastPositionTool1 = m_ToolVisualizationFilter->GetOutput(0)->GetPosition();
      lastTimeStamp = m_ToolVisualizationFilter->GetOutput(0)->GetTimeStamp();
      }
    }
}

void QmitkMITKIGTTrackingToolboxView::OnChooseFileClicked()
{
  QDir currentPath = QFileInfo(m_Controls->m_LoggingFileName->text()).dir();

  // if no path was selected (QDir would select current working dir then) or the
  // selected path does not exist -> use home directory
  if ( currentPath == QDir() || ! currentPath.exists() )
  {
    currentPath = QDir(QDir::homePath());
  }

  QString filename = QFileDialog::getSaveFileName(NULL,tr("Choose Logging File"), currentPath.absolutePath(), "*.*");
  if (filename == "") return;
  this->m_Controls->m_LoggingFileName->setText(filename);
  this->OnToggleFileExtension();
}
// bug-16470: toggle file extension after clicking on radio button
void QmitkMITKIGTTrackingToolboxView::OnToggleFileExtension()
{
  QString currentInputText = this->m_Controls->m_LoggingFileName->text();
  QString currentFile = QFileInfo(currentInputText).baseName();
  QDir currentPath = QFileInfo(currentInputText).dir();
  if(currentFile.isEmpty())
  {
    currentFile = "logfile";
  }
  // Setting currentPath to default home path when currentPath is empty or it does not exist
  if(currentPath == QDir() || !currentPath.exists())
  {
    currentPath = QDir::homePath();
  }
  // check if csv radio button is clicked
  if(this->m_Controls->m_csvFormat->isChecked())
  {
    // you needn't add a seperator to the input text when currentpath is the rootpath
    if(currentPath.isRoot())
    {
      this->m_Controls->m_LoggingFileName->setText(QDir::toNativeSeparators(currentPath.absolutePath()) + currentFile + ".csv");
    }

    else
    {
      this->m_Controls->m_LoggingFileName->setText(QDir::toNativeSeparators(currentPath.absolutePath()) + QDir::separator() + currentFile + ".csv");
    }
  }
  // check if xml radio button is clicked
  else if(this->m_Controls->m_xmlFormat->isChecked())
  {
    // you needn't add a seperator to the input text when currentpath is the rootpath
    if(currentPath.isRoot())
    {
      this->m_Controls->m_LoggingFileName->setText(QDir::toNativeSeparators(currentPath.absolutePath()) + currentFile + ".xml");
    }
    else
    {
      this->m_Controls->m_LoggingFileName->setText(QDir::toNativeSeparators(currentPath.absolutePath()) + QDir::separator() + currentFile + ".xml");
    }
  }
}

void QmitkMITKIGTTrackingToolboxView::StartLogging()
{
  if (m_ToolVisualizationFilter.IsNull())
  {
    MessageBox("Cannot activate logging without a connected device. Configure and connect a tracking device first.");
    return;
  }

  if (!m_logging)
  {
    //initialize logging filter
    m_loggingFilter = mitk::NavigationDataRecorder::New();

    m_loggingFilter->ConnectTo(m_ToolVisualizationFilter);

    if (m_Controls->m_LoggingLimit->isChecked()){m_loggingFilter->SetRecordCountLimit(m_Controls->m_LoggedFramesLimit->value());}

    //start filter with try-catch block for exceptions
    try
    {
      m_loggingFilter->StartRecording();
    }
    catch(mitk::IGTException)
    {
      std::string errormessage = "Error during start recording. Recorder already started recording?";
      QMessageBox::warning(NULL, "IGTPlayer: Error", errormessage.c_str());
      m_loggingFilter->StopRecording();
      return;
    }

    //update labels / logging variables
    this->m_Controls->m_LoggingLabel->setText("Logging ON");
    this->m_Controls->m_LoggedFramesLabel->setText("Logged Frames: 0");
    m_loggedFrames = 0;
    m_logging = true;
    DisableLoggingButtons();
  }
}

void QmitkMITKIGTTrackingToolboxView::StopLogging()
{
  if (m_logging)
  {
    //stop logging
    m_loggingFilter->StopRecording();
    m_logging = false;

    //update GUI
    this->m_Controls->m_LoggingLabel->setText("Logging OFF");
    EnableLoggingButtons();

    //write the results to a file
    if(m_Controls->m_csvFormat->isChecked())
      {
      mitk::NavigationDataSetWriterCSV* writer = new mitk::NavigationDataSetWriterCSV();
      writer->Write(this->m_Controls->m_LoggingFileName->text().toStdString(),m_loggingFilter->GetNavigationDataSet());
      delete writer;
      }
    else if (m_Controls->m_xmlFormat->isChecked())
      {
      mitk::NavigationDataSetWriterXML* writer = new mitk::NavigationDataSetWriterXML();
      writer->Write(this->m_Controls->m_LoggingFileName->text().toStdString(),m_loggingFilter->GetNavigationDataSet());
      delete writer;
      }
  }
}

void QmitkMITKIGTTrackingToolboxView::OnAddSingleTool()
{
  QString Identifier = "Tool#";
  if (m_toolStorage.IsNotNull()) Identifier += QString::number(m_toolStorage->GetToolCount());
  else Identifier += "0";
  m_Controls->m_NavigationToolCreationWidget->Initialize(GetDataStorage(),Identifier.toStdString());
  m_Controls->m_NavigationToolCreationWidget->SetTrackingDeviceType(m_Controls->m_configurationWidget->GetTrackingDevice()->GetType(),false);
  m_Controls->m_TrackingToolsWidget->setCurrentIndex(1);

  //disable tracking volume during tool editing
  lastTrackingVolumeState = m_Controls->m_ShowTrackingVolume->isChecked();
  if (lastTrackingVolumeState) m_Controls->m_ShowTrackingVolume->click();
  GlobalReinit();
}

void QmitkMITKIGTTrackingToolboxView::OnAddSingleToolFinished()
{
  m_Controls->m_TrackingToolsWidget->setCurrentIndex(0);
  if (this->m_toolStorage.IsNull())
  {
    //this shouldn't happen!
    MITK_WARN << "No ToolStorage available, cannot add tool, aborting!";
    return;
  }
  m_toolStorage->AddTool(m_Controls->m_NavigationToolCreationWidget->GetCreatedTool());
  m_Controls->m_TrackingToolsStatusWidget->PreShowTools(m_toolStorage);
  QString toolLabel = QString("Loaded Tools: <manually added>");

  //enable tracking volume again
  if (lastTrackingVolumeState) m_Controls->m_ShowTrackingVolume->click();
  GlobalReinit();
}

void QmitkMITKIGTTrackingToolboxView::OnAddSingleToolCanceled()
{
  m_Controls->m_TrackingToolsWidget->setCurrentIndex(0);

  //enable tracking volume again
  if (lastTrackingVolumeState) m_Controls->m_ShowTrackingVolume->click();
  GlobalReinit();
}

void QmitkMITKIGTTrackingToolboxView::GlobalReinit()
{
  // get all nodes that have not set "includeInBoundingBox" to false
  mitk::NodePredicateNot::Pointer pred = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("includeInBoundingBox", mitk::BoolProperty::New(false)));

  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetDataStorage()->GetSubset(pred);
  // calculate bounding geometry of these nodes
  mitk::TimeGeometry::Pointer bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(rs, "visible");

  // initialize the views to the bounding geometry
  mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
}

void QmitkMITKIGTTrackingToolboxView::DisableLoggingButtons()
{
  m_Controls->m_StartLogging->setEnabled(false);
  m_Controls->m_LoggingFileName->setEnabled(false);
  m_Controls->m_ChooseFile->setEnabled(false);
  m_Controls->m_LoggingLimit->setEnabled(false);
  m_Controls->m_LoggedFramesLimit->setEnabled(false);
  m_Controls->m_csvFormat->setEnabled(false);
  m_Controls->m_xmlFormat->setEnabled(false);
  m_Controls->m_StopLogging->setEnabled(true);
}

void QmitkMITKIGTTrackingToolboxView::EnableLoggingButtons()
{
  m_Controls->m_StartLogging->setEnabled(true);
  m_Controls->m_LoggingFileName->setEnabled(true);
  m_Controls->m_ChooseFile->setEnabled(true);
  m_Controls->m_LoggingLimit->setEnabled(true);
  m_Controls->m_LoggedFramesLimit->setEnabled(true);
  m_Controls->m_csvFormat->setEnabled(true);
  m_Controls->m_xmlFormat->setEnabled(true);
  m_Controls->m_StopLogging->setEnabled(false);
}

void QmitkMITKIGTTrackingToolboxView::DisableOptionsButtons()
{
  m_Controls->m_ShowTrackingVolume->setEnabled(false);
  m_Controls->m_UpdateRate->setEnabled(false);
  m_Controls->m_ShowToolQuaternions->setEnabled(false);
  m_Controls->m_OptionsUpdateRateLabel->setEnabled(false);
}

void QmitkMITKIGTTrackingToolboxView::EnableOptionsButtons()
{
  m_Controls->m_ShowTrackingVolume->setEnabled(true);
  m_Controls->m_UpdateRate->setEnabled(true);
  m_Controls->m_ShowToolQuaternions->setEnabled(true);
  m_Controls->m_OptionsUpdateRateLabel->setEnabled(true);
}

void QmitkMITKIGTTrackingToolboxView::EnableTrackingControls()
{
  m_Controls->m_TrackingControlsGroupBox->setEnabled(true);
}

void QmitkMITKIGTTrackingToolboxView::DisableTrackingControls()
{
  m_Controls->m_TrackingControlsGroupBox->setEnabled(false);
}

void QmitkMITKIGTTrackingToolboxView::EnableTrackingConfigurationButtons()
{
  m_Controls->m_AutoDetectTools->setEnabled(true);
  if (m_Controls->m_configurationWidget->GetTrackingDevice()->GetType() != mitk::NDIAurora) m_Controls->m_AddSingleTool->setEnabled(true);
  m_Controls->m_LoadTools->setEnabled(true);
  m_Controls->m_ResetTools->setEnabled(true);
}

void QmitkMITKIGTTrackingToolboxView::DisableTrackingConfigurationButtons()
{
  m_Controls->m_AutoDetectTools->setEnabled(false);
  if (m_Controls->m_configurationWidget->GetTrackingDevice()->GetType() != mitk::NDIAurora) m_Controls->m_AddSingleTool->setEnabled(false);
  m_Controls->m_LoadTools->setEnabled(false);
  m_Controls->m_ResetTools->setEnabled(false);
}

void QmitkMITKIGTTrackingToolboxView::ReplaceCurrentToolStorage(mitk::NavigationToolStorage::Pointer newStorage, std::string newStorageName)
{
  //first: get rid of the old one
  //don't reset if there is no tool storage. BugFix #17793
  if ( m_toolStorage.IsNotNull() ){
    m_toolStorage->UnLockStorage(); //only to be sure...
    m_toolStorage->UnRegisterMicroservice();
    m_toolStorage = NULL;
  }

  //now: replace by the new one
  m_toolStorage = newStorage;
  m_toolStorage->SetName(newStorageName);
  m_toolStorage->RegisterAsMicroservice("no tracking device");
}

void QmitkMITKIGTTrackingToolboxView::OnTimeOut()
{
  MITK_INFO << "Time Out";
  m_WorkerThread->terminate();
  m_WorkerThread->wait();

  m_TimeoutTimer->stop();
}

void QmitkMITKIGTTrackingToolboxView::StoreUISettings()
{
  // persistence service does not directly work in plugins for now
  // -> using QSettings
  QSettings settings;

  settings.beginGroup(QString::fromStdString(VIEW_ID));

  // set the values of some widgets and attrbutes to the QSettings
  settings.setValue("ShowTrackingVolume", QVariant(m_Controls->m_ShowTrackingVolume->isChecked()));
  settings.setValue("toolStorageFilename", QVariant(m_ToolStorageFilename));
  settings.setValue("VolumeSelectionBox", QVariant(m_Controls->m_VolumeSelectionBox->currentIndex()));

  settings.endGroup();
}

void QmitkMITKIGTTrackingToolboxView::LoadUISettings()
{
  // persistence service does not directly work in plugins for now
  // -> using QSettings
  QSettings settings;

  settings.beginGroup(QString::fromStdString(VIEW_ID));

  // set some widgets and attributes by the values from the QSettings
  m_Controls->m_ShowTrackingVolume->setChecked(settings.value("ShowTrackingVolume", true).toBool());
  m_Controls->m_VolumeSelectionBox->setCurrentIndex(settings.value("VolumeSelectionBox", 0).toInt());
  m_ToolStorageFilename = settings.value("toolStorageFilename", QVariant("")).toString();

  settings.endGroup();


  // try to deserialize the tool storage from the given tool storage file name
  if ( ! m_ToolStorageFilename.isEmpty() )
  {
    // try-catch block for exceptions
    try
    {
      mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(GetDataStorage());
      m_toolStorage->UnRegisterMicroservice();
      m_toolStorage = myDeserializer->Deserialize(m_ToolStorageFilename.toStdString());
      m_toolStorage->RegisterAsMicroservice("no tracking device");

      //update label
      UpdateToolStorageLabel(m_ToolStorageFilename);

      //update tool preview
      m_Controls->m_TrackingToolsStatusWidget->RemoveStatusLabels();
      m_Controls->m_TrackingToolsStatusWidget->PreShowTools(m_toolStorage);
    }
    catch(mitk::IGTException)
    {
      MITK_WARN("QmitkMITKIGTTrackingToolBoxView") << "Error during restoring tools. Problems with file ("<<m_ToolStorageFilename.toStdString()<<"), please check the file?";
      this->OnResetTools(); //if there where errors reset the tool storage to avoid problems later on
    }
  }
}

void QmitkMITKIGTTrackingToolboxView::UpdateToolStorageLabel(QString pathOfLoadedStorage)
{
  Poco::Path myPath = Poco::Path(pathOfLoadedStorage.toStdString()); //use this to seperate filename from path
  QString toolLabel = QString("Loaded ") + QString::number(m_toolStorage->GetToolCount()) + " Tools from " + myPath.getFileName().c_str();
  if (toolLabel.size() > 55) //if the tool storage name is to long trimm the string
    {
    toolLabel.resize(50);
    toolLabel+="[...]";
    }
  m_Controls->m_toolLabel->setText(toolLabel);
}


void QmitkMITKIGTTrackingToolboxViewWorker::SetWorkerMethod(WorkerMethod w)
{
  m_WorkerMethod = w;
}

void QmitkMITKIGTTrackingToolboxViewWorker::SetTrackingDevice(mitk::TrackingDevice::Pointer t)
{
  m_TrackingDevice = t;
}

void QmitkMITKIGTTrackingToolboxViewWorker::SetDataStorage(mitk::DataStorage::Pointer d)
{
  m_DataStorage = d;
}

void QmitkMITKIGTTrackingToolboxViewWorker::SetInverseMode(bool mode)
{
  m_InverseMode = mode;
}

void QmitkMITKIGTTrackingToolboxViewWorker::SetTrackingDeviceData(mitk::TrackingDeviceData d)
{
  m_TrackingDeviceData = d;
}

void QmitkMITKIGTTrackingToolboxViewWorker::SetNavigationToolStorage(mitk::NavigationToolStorage::Pointer n)
{
  m_NavigationToolStorage = n;
}

void QmitkMITKIGTTrackingToolboxViewWorker::ThreadFunc()
{
  switch(m_WorkerMethod)
  {
  case eAutoDetectTools:
    this->AutoDetectTools();
    break;
  case eConnectDevice:
    this->ConnectDevice();
    break;
  case eStartTracking:
    this->StartTracking();
    break;
  case eStopTracking:
    this->StopTracking();
    break;
  case eDisconnectDevice:
    this->DisconnectDevice();
    break;
  default:
    MITK_WARN << "Undefined worker method was set ... something went wrong!";
    break;
    }
}

void QmitkMITKIGTTrackingToolboxViewWorker::AutoDetectTools()
{
  mitk::ProgressBar::GetInstance()->AddStepsToDo(4);
  mitk::NavigationToolStorage::Pointer autoDetectedStorage = mitk::NavigationToolStorage::New(m_DataStorage);
  mitk::NDITrackingDevice::Pointer currentDevice = dynamic_cast<mitk::NDITrackingDevice*>(m_TrackingDevice.GetPointer());
  try
  {
    currentDevice->OpenConnection();
    mitk::ProgressBar::GetInstance()->Progress();
    currentDevice->StartTracking();
  }
  catch(mitk::Exception& e)
  {
    QString message = QString("Warning, can not auto-detect tools! (") + QString(e.GetDescription()) + QString(")");
    //MessageBox(message.toStdString()); //TODO: give message to the user here!

    MITK_WARN << message.toStdString();
    mitk::ProgressBar::GetInstance()->Progress(4);
    emit AutoDetectToolsFinished(false,message.toStdString().c_str());
    return;
  }

  for (int i=0; i<currentDevice->GetToolCount(); i++)
  {
    //create a navigation tool with sphere as surface
    std::stringstream toolname;
    toolname << "AutoDetectedTool" << i;
    mitk::NavigationTool::Pointer newTool = mitk::NavigationTool::New();
    newTool->SetSerialNumber(dynamic_cast<mitk::NDIPassiveTool*>(currentDevice->GetTool(i))->GetSerialNumber());
    newTool->SetIdentifier(toolname.str());
    newTool->SetTrackingDeviceType(mitk::NDIAurora);
    mitk::DataNode::Pointer newNode = mitk::DataNode::New();
    mitk::Surface::Pointer mySphere = mitk::Surface::New();
    vtkSphereSource *vtkData = vtkSphereSource::New();
    vtkData->SetRadius(3.0f);
    vtkData->SetCenter(0.0, 0.0, 0.0);
    vtkData->Update();
    mySphere->SetVtkPolyData(vtkData->GetOutput());
    vtkData->Delete();
    newNode->SetData(mySphere);
    newNode->SetName(toolname.str());
    newTool->SetDataNode(newNode);
    autoDetectedStorage->AddTool(newTool);
  }

  m_NavigationToolStorage = autoDetectedStorage;

  currentDevice->StopTracking();
  mitk::ProgressBar::GetInstance()->Progress();
  currentDevice->CloseConnection();

  emit AutoDetectToolsFinished(true,"");
  mitk::ProgressBar::GetInstance()->Progress(4);
}

void QmitkMITKIGTTrackingToolboxViewWorker::ConnectDevice()
{
  std::string message = "";
  mitk::ProgressBar::GetInstance()->AddStepsToDo(10);

  //build the IGT pipeline
  mitk::TrackingDevice::Pointer trackingDevice = m_TrackingDevice;
  trackingDevice->SetData(m_TrackingDeviceData);

  //set device to rotation mode transposed becaus we are working with VNL style quaternions
  if(m_InverseMode)
  {trackingDevice->SetRotationMode(mitk::TrackingDevice::RotationTransposed);}

  //Get Tracking Volume Data
  mitk::TrackingDeviceData data = m_TrackingDeviceData;
  mitk::ProgressBar::GetInstance()->Progress();

  //Create Navigation Data Source with the factory class
  mitk::TrackingDeviceSourceConfigurator::Pointer myTrackingDeviceSourceFactory = mitk::TrackingDeviceSourceConfigurator::New(m_NavigationToolStorage,trackingDevice);
  m_TrackingDeviceSource = myTrackingDeviceSourceFactory->CreateTrackingDeviceSource(m_ToolVisualizationFilter);
  mitk::ProgressBar::GetInstance()->Progress();

  if ( m_TrackingDeviceSource.IsNull() )
  {
    message = std::string("Cannot connect to device: ") + myTrackingDeviceSourceFactory->GetErrorMessage();
    emit ConnectDeviceFinished(false,QString(message.c_str()));
    return;
  }

  //set filter to rotation mode transposed becaus we are working with VNL style quaternions
  if(m_InverseMode)
    m_ToolVisualizationFilter->SetRotationMode(mitk::NavigationDataObjectVisualizationFilter::RotationTransposed);

  //First check if the created object is valid
  if (m_TrackingDeviceSource.IsNull())
  {
    message = myTrackingDeviceSourceFactory->GetErrorMessage();
    emit ConnectDeviceFinished(false,QString(message.c_str()));
    return;
  }

  MITK_INFO << "Number of tools: " << m_TrackingDeviceSource->GetNumberOfOutputs();
  mitk::ProgressBar::GetInstance()->Progress();

  //The tools are maybe reordered after initialization, e.g. in case of auto-detected tools of NDI Aurora
  mitk::NavigationToolStorage::Pointer toolsInNewOrder = myTrackingDeviceSourceFactory->GetUpdatedNavigationToolStorage();

  if ((toolsInNewOrder.IsNotNull()) && (toolsInNewOrder->GetToolCount() > 0))
  {
    //so delete the old tools in wrong order and add them in the right order
    //we cannot simply replace the tool storage because the new storage is
    //not correctly initialized with the right data storage

    /*
    m_NavigationToolStorage->DeleteAllTools();
    for (int i=0; i < toolsInNewOrder->GetToolCount(); i++) {m_NavigationToolStorage->AddTool(toolsInNewOrder->GetTool(i));}

    This was replaced and thereby fixed Bug 18318 DeleteAllTools() is not Threadsafe!
    */
    for(int i = 0; i < toolsInNewOrder->GetToolCount(); i++ )
    {
      m_NavigationToolStorage->AssignToolNumber(toolsInNewOrder->GetTool(i)->GetIdentifier(),i);
    }

  }

  mitk::ProgressBar::GetInstance()->Progress();

  //connect to device
  try
  {
    m_TrackingDeviceSource->Connect();
    mitk::ProgressBar::GetInstance()->Progress();
    //Microservice registration:
    m_TrackingDeviceSource->RegisterAsMicroservice();
    m_NavigationToolStorage->UnRegisterMicroservice();
    m_NavigationToolStorage->RegisterAsMicroservice(m_TrackingDeviceSource->GetMicroserviceID());
    m_NavigationToolStorage->LockStorage();
  }
  catch (...) //todo: change to mitk::IGTException
  {
    message = "Error on connecting the tracking device.";
    emit ConnectDeviceFinished(false,QString(message.c_str()));
    return;
  }
  emit ConnectDeviceFinished(true,QString(message.c_str()));
  mitk::ProgressBar::GetInstance()->Progress(10);
}

void QmitkMITKIGTTrackingToolboxViewWorker::StartTracking()
{
  QString errorMessage = "";
  try
  {
    m_TrackingDeviceSource->StartTracking();
  }
  catch (...) //todo: change to mitk::IGTException
  {
    errorMessage += "Error while starting the tracking device!";
    emit StartTrackingFinished(false,errorMessage);
    return;
  }
  //remember the original colors of the tools
  m_OriginalColors = std::map<mitk::DataNode::Pointer,mitk::Color>();
  for(int i=0; i<this->m_NavigationToolStorage->GetToolCount(); i++)
    {
    mitk::DataNode::Pointer currentToolNode = m_NavigationToolStorage->GetTool(i)->GetDataNode();
    float c[3];
    currentToolNode->GetColor(c);
    mitk::Color color;
    color.SetRed(c[0]);
    color.SetGreen(c[1]);
    color.SetBlue(c[2]);
    m_OriginalColors[currentToolNode] = color;
    }

  emit StartTrackingFinished(true,errorMessage);
}

void QmitkMITKIGTTrackingToolboxViewWorker::StopTracking()
{
  //stop tracking
  try
  {
    m_TrackingDeviceSource->StopTracking();
  }
  catch(mitk::Exception& e)
  {
    emit StopTrackingFinished(false, e.GetDescription());
  }

  //restore the original colors of the tools
  for(int i=0; i<this->m_NavigationToolStorage->GetToolCount(); i++)
  {
    mitk::DataNode::Pointer currentToolNode = m_NavigationToolStorage->GetTool(i)->GetDataNode();
    if (m_OriginalColors.find(currentToolNode) == m_OriginalColors.end())
      {MITK_WARN << "Cannot restore original color of tool " << m_NavigationToolStorage->GetTool(i)->GetToolName();}
    else
      {currentToolNode->SetColor(m_OriginalColors[currentToolNode]);}
  }

  //emit signal
  emit StopTrackingFinished(true, "");
}

void QmitkMITKIGTTrackingToolboxViewWorker::DisconnectDevice()
{
  try
  {
    if (m_TrackingDeviceSource->IsTracking()) {m_TrackingDeviceSource->StopTracking();}
    m_TrackingDeviceSource->Disconnect();
    m_TrackingDeviceSource->UnRegisterMicroservice();
    m_NavigationToolStorage->UnLockStorage();
  }
  catch(mitk::Exception& e)
  {
    emit DisconnectDeviceFinished(false, e.GetDescription());
  }
  emit DisconnectDeviceFinished(true, "");
}
