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
  m_tracking = false;
  m_logging = false;
  m_loggedFrames = 0;

  //initialize worker thread
  m_WorkerThread = new QThread();
  m_Worker = new QmitkMITKIGTTrackingToolboxViewWorker();
}

QmitkMITKIGTTrackingToolboxView::~QmitkMITKIGTTrackingToolboxView()
{
//clean up worker thread
if(m_WorkerThread) delete m_WorkerThread;
if(m_Worker) delete m_Worker;
//remove the tracking volume
this->GetDataStorage()->Remove(m_TrackingVolumeNode);
//remove the tool storage
m_toolStorage->UnRegisterMicroservice();
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
    connect( m_Controls->m_Connect, SIGNAL(clicked()), this, SLOT(OnConnect()) );
    connect( m_Controls->m_Disconnect, SIGNAL(clicked()), this, SLOT(OnDisconnect()) );
    connect( m_Controls->m_StartTracking, SIGNAL(clicked()), this, SLOT(OnStartTracking()) );
    connect( m_Controls->m_StopTracking, SIGNAL(clicked()), this, SLOT(OnStopTracking()) );
    connect( m_TrackingTimer, SIGNAL(timeout()), this, SLOT(UpdateTrackingTimer()));
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
    connect(m_Worker, SIGNAL(AutoDetectToolsFinished()), this, SLOT(OnAutoDetectToolsFinished()) );
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
    m_TrackingVolumeNode->SetOpacity(0.25);
    m_TrackingVolumeNode->SetBoolProperty("Backface Culling",true);
    mitk::Color red;
    red.SetRed(1);
    m_TrackingVolumeNode->SetColor(red);
    GetDataStorage()->Add(m_TrackingVolumeNode);

    //initialize buttons
    m_Controls->m_Connect->setEnabled(true);
    m_Controls->m_Disconnect->setEnabled(false);
    m_Controls->m_StartTracking->setEnabled(false);
    m_Controls->m_StopTracking->setEnabled(false);
    m_Controls->m_AutoDetectTools->setVisible(false); //only visible if tracking device is Aurora

    //Update List of available models for selected tool.
    std::vector<mitk::TrackingDeviceData> Compatibles = mitk::GetDeviceDataForLine( m_Controls->m_configurationWidget->GetTrackingDevice()->GetType());
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
   std::string errormessage = "Error during deserializing. Problems with file,please check the file?";
   QMessageBox::warning(NULL, "IGTPlayer: Error", errormessage.c_str());
   return;
  }

  if(m_toolStorage->isEmpty())
    {
    errorMessage = myDeserializer->GetErrorMessage();
    MessageBox(errorMessage);
    return;
    }

  //update label
  Poco::Path myPath = Poco::Path(filename.toStdString()); //use this to seperate filename from path
  QString toolLabel = QString("Loaded Tools: ") + QString::number(m_toolStorage->GetToolCount()) + " Tools from " + myPath.getFileName().c_str();
  m_Controls->m_toolLabel->setText(toolLabel);

  //update tool preview
  m_Controls->m_TrackingToolsStatusWidget->RemoveStatusLabels();
  m_Controls->m_TrackingToolsStatusWidget->PreShowTools(m_toolStorage);
}

void QmitkMITKIGTTrackingToolboxView::OnResetTools()
{
  this->ReplaceCurrentToolStorage(mitk::NavigationToolStorage::New(GetDataStorage()),"TrackingToolbox Default Storage");
  m_Controls->m_TrackingToolsStatusWidget->RemoveStatusLabels();
  QString toolLabel = QString("Loaded Tools: <none>");
  m_Controls->m_toolLabel->setText(toolLabel);
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
  m_Controls->m_Disconnect->setEnabled(true);
  m_Controls->m_StartTracking->setEnabled(true);
  m_Controls->m_StopTracking->setEnabled(false);
  m_Controls->m_Connect->setEnabled(false);
  DisableOptionsButtons();
  DisableTrackingConfigurationButtons();
  m_Controls->m_configurationWidget->ConfigurationFinished();

  m_Controls->m_TrackingControlLabel->setText("Status: connected");
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
  m_Controls->m_MainWidget->setEnabled(true);

  if (!success)
    {
    MITK_WARN << errorMessage.toStdString();
    MessageBox(errorMessage.toStdString());
    return;
    }

  //enable/disable Buttons
  m_Controls->m_Disconnect->setEnabled(false);
  m_Controls->m_StartTracking->setEnabled(false);
  m_Controls->m_StopTracking->setEnabled(false);
  m_Controls->m_Connect->setEnabled(true);
  EnableOptionsButtons();
  EnableTrackingConfigurationButtons();
  m_Controls->m_configurationWidget->Reset();
  m_Controls->m_TrackingControlLabel->setText("Status: disconnected");
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

  //enable/disable Buttons
  m_Controls->m_Disconnect->setEnabled(true);
  m_Controls->m_StartTracking->setEnabled(false);
  m_Controls->m_StopTracking->setEnabled(true);
  m_Controls->m_Connect->setEnabled(false);

  m_tracking = true;

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

  //enable/disable Buttons
  m_Controls->m_Disconnect->setEnabled(true);
  m_Controls->m_StartTracking->setEnabled(true);
  m_Controls->m_StopTracking->setEnabled(false);
  m_Controls->m_Connect->setEnabled(false);

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
  if (m_Controls->m_ShowTrackingVolume->isChecked())
  {
    mitk::TrackingVolumeGenerator::Pointer volumeGenerator = mitk::TrackingVolumeGenerator::New();

    std::string str = qstr.toStdString();

    mitk::TrackingDeviceData data = mitk::GetDeviceDataByName(str);
    m_TrackingDeviceData = data;

    volumeGenerator->SetTrackingDeviceData(data);
    volumeGenerator->Update();

    mitk::Surface::Pointer volumeSurface = volumeGenerator->GetOutput();

    m_TrackingVolumeNode->SetData(volumeSurface);

    GlobalReinit();
  }
}

void QmitkMITKIGTTrackingToolboxView::OnShowTrackingVolumeChanged()
{
  if (m_Controls->m_ShowTrackingVolume->isChecked())
    {
    OnTrackingVolumeChanged(m_Controls->m_VolumeSelectionBox->currentText());
    GetDataStorage()->Add(m_TrackingVolumeNode);
    }
  else
    {
    GetDataStorage()->Remove(m_TrackingVolumeNode);
    GlobalReinit();
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
    //disable controls until worker thread is finished
    this->m_Controls->m_MainWidget->setEnabled(false);
    }
}

void QmitkMITKIGTTrackingToolboxView::OnAutoDetectToolsFinished()
{
    m_WorkerThread->quit();

    //enable controls again
    this->m_Controls->m_MainWidget->setEnabled(true);

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
        QMessageBox::warning(NULL, "IGTPlayer: Error", errormessage.c_str());}
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
  m_ToolVisualizationFilter->Update();
  MITK_DEBUG << "Number of outputs ToolVisualizationFilter: " << m_ToolVisualizationFilter->GetNumberOfIndexedOutputs();
  MITK_DEBUG << "Number of inputs ToolVisualizationFilter: " << m_ToolVisualizationFilter->GetNumberOfIndexedInputs();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  if (m_logging)
    {
    this->m_loggingFilter->Update();
    m_loggedFrames = this->m_loggingFilter->GetRecordCounter();
    this->m_Controls->m_LoggedFramesLabel->setText("Logged Frames: "+QString::number(m_loggedFrames));
    //check if logging stopped automatically
    if((m_loggedFrames>1)&&(!m_loggingFilter->GetRecording())){StopLogging();}
    }
  m_Controls->m_TrackingToolsStatusWidget->Refresh();
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
    m_loggingFilter->SetRecordingMode(mitk::NavigationDataRecorder::NormalFile);
    if (m_Controls->m_xmlFormat->isChecked()) m_loggingFilter->SetOutputFormat(mitk::NavigationDataRecorder::xml);
    else if (m_Controls->m_csvFormat->isChecked()) m_loggingFilter->SetOutputFormat(mitk::NavigationDataRecorder::csv);
    std::string filename = m_Controls->m_LoggingFileName->text().toStdString().c_str();
    // this part has been changed in order to prevent crash of the  program
    if(!filename.empty())
    m_loggingFilter->SetFileName(filename);
    else if(filename.empty()){
     std::string errormessage = "File name has not been set, please set the file name";
     mitkThrowException(mitk::IGTIOException)<<errormessage;
     QMessageBox::warning(NULL, "IGTPlayer: Error", errormessage.c_str());
     m_loggingFilter->SetFileName(filename);
    }

    if (m_Controls->m_LoggingLimit->isChecked()){m_loggingFilter->SetRecordCountLimit(m_Controls->m_LoggedFramesLimit->value());}

    //connect filter
    for(int i=0; i<m_ToolVisualizationFilter->GetNumberOfOutputs(); i++){m_loggingFilter->AddNavigationData(m_ToolVisualizationFilter->GetOutput(i));}

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
    //update label
    this->m_Controls->m_LoggingLabel->setText("Logging OFF");

    m_loggingFilter->StopRecording();
    m_logging = false;
    EnableLoggingButtons();
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
    m_toolStorage->UnLockStorage(); //only to be sure...
    m_toolStorage->UnRegisterMicroservice();
    m_toolStorage = NULL;

    //now: replace by the new one
    m_toolStorage = newStorage;
    m_toolStorage->SetName(newStorageName);
    m_toolStorage->RegisterAsMicroservice("no tracking device");
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
  currentDevice->OpenConnection();
  mitk::ProgressBar::GetInstance()->Progress();
  currentDevice->StartTracking();
  mitk::ProgressBar::GetInstance()->Progress();

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

  emit AutoDetectToolsFinished();
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
    m_NavigationToolStorage->DeleteAllTools();
    for (int i=0; i < toolsInNewOrder->GetToolCount(); i++) {m_NavigationToolStorage->AddTool(toolsInNewOrder->GetTool(i));}
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
  emit StartTrackingFinished(true,errorMessage);
}

void QmitkMITKIGTTrackingToolboxViewWorker::StopTracking()
{
  try
  {
  m_TrackingDeviceSource->StopTracking();
  }
  catch(mitk::Exception& e)
  {
    emit StopTrackingFinished(false, e.GetDescription());
  }
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
