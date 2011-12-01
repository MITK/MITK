/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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

// vtk
#include <vtkSphereSource.h>


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
}

QmitkMITKIGTTrackingToolboxView::~QmitkMITKIGTTrackingToolboxView()
{
//remove the tracking volume
this->GetDataStorage()->Remove(m_TrackingVolumeNode);
this->GlobalReinit();
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
    connect( m_Controls->m_StartTracking, SIGNAL(clicked()), this, SLOT(OnStartTracking()) );
    connect( m_Controls->m_StopTracking, SIGNAL(clicked()), this, SLOT(OnStopTracking()) );
    connect( m_TrackingTimer, SIGNAL(timeout()), this, SLOT(UpdateTrackingTimer()));
    connect( m_Controls->m_ChooseFile, SIGNAL(clicked()), this, SLOT(OnChooseFileClicked()));
    connect( m_Controls->m_StartLogging, SIGNAL(clicked()), this, SLOT(StartLogging()));
    connect( m_Controls->m_StopLogging, SIGNAL(clicked()), this, SLOT(StopLogging()));
    connect( m_Controls->m_configurationWidget, SIGNAL(TrackingDeviceSelectionChanged()), this, SLOT(OnTrackingDeviceChanged()));
	  connect( m_Controls->m_VolumeSelectionBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(OnTrackingVolumeChanged(QString)));
    connect( m_Controls->m_ShowTrackingVolume, SIGNAL(clicked()), this, SLOT(OnShowTrackingVolumeChanged()));
    connect( m_Controls->m_AutoDetectTools, SIGNAL(clicked()), this, SLOT(OnAutoDetectTools()));
    connect( m_Controls->m_ResetTools, SIGNAL(clicked()), this, SLOT(OnResetTools()));

    connect( m_Controls->m_AddSingleTool, SIGNAL(clicked()), this, SLOT(OnAddSingleTool()));
    connect( m_Controls->m_NavigationToolCreationWidget, SIGNAL(NavigationToolFinished()), this, SLOT(OnAddSingleToolFinished()));
    connect( m_Controls->m_NavigationToolCreationWidget, SIGNAL(Canceled()), this, SLOT(OnAddSingleToolCanceled()));

    //initialize widgets
    m_Controls->m_configurationWidget->EnableAdvancedUserControl(false);
    m_Controls->m_TrackingToolsStatusWidget->SetShowPositions(true);
    m_Controls->m_TrackingToolsStatusWidget->SetTextAlignment(Qt::AlignLeft);

    //initialize tracking volume node
    m_TrackingVolumeNode = mitk::DataNode::New();
    m_TrackingVolumeNode->SetName("TrackingVolume");
    m_TrackingVolumeNode->SetOpacity(0.25);
    mitk::Color red;
    red.SetRed(1);
    m_TrackingVolumeNode->SetColor(red);
    GetDataStorage()->Add(m_TrackingVolumeNode);

    //initialize buttons
    m_Controls->m_StopTracking->setEnabled(false);
    m_Controls->m_StopLogging->setEnabled(false);
    m_Controls->m_AutoDetectTools->setVisible(false); //only visible if tracking device is Aurora

	  //Update List of available models for selected tool.
    std::vector<mitk::TrackingDeviceData> Compatibles = mitk::GetDeviceDataForLine( m_Controls->m_configurationWidget->GetTrackingDevice()->GetType());
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
  QString filename = QFileDialog::getOpenFileName(NULL,tr("Open Toolfile"), "/", tr("All Files (*.*)")); //later perhaps: tr("Toolfile (*.tfl)"
  if (filename.isNull()) return;

  //read tool storage from disk
  mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(GetDataStorage());
  m_toolStorage = myDeserializer->Deserialize(filename.toStdString());
  if (m_toolStorage.IsNull())
    {
    MessageBox(myDeserializer->GetErrorMessage());
    m_toolStorage = NULL;
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
  m_toolStorage = NULL;
  m_Controls->m_TrackingToolsStatusWidget->RemoveStatusLabels();
  QString toolLabel = QString("Loaded Tools: <none>");
  m_Controls->m_toolLabel->setText(toolLabel);
}

void QmitkMITKIGTTrackingToolboxView::OnStartTracking()
{
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

//build the IGT pipeline
mitk::TrackingDevice::Pointer trackingDevice = this->m_Controls->m_configurationWidget->GetTrackingDevice();

//Get Tracking Volume Data
mitk::TrackingDeviceData data = mitk::DeviceDataUnspecified;

QString qstr =  m_Controls->m_VolumeSelectionBox->currentText();
if ( (! qstr.isNull()) || (! qstr.isEmpty()) ) {
  std::string str = qstr.toStdString();
  data = mitk::GetDeviceDataByName(str); //Data will be set later, after device generation
}

mitk::TrackingDeviceSourceConfigurator::Pointer myTrackingDeviceSourceFactory = mitk::TrackingDeviceSourceConfigurator::New(this->m_toolStorage,trackingDevice);
m_TrackingDeviceSource = myTrackingDeviceSourceFactory->CreateTrackingDeviceSource(this->m_ToolVisualizationFilter);
if (m_TrackingDeviceSource.IsNull())
  {
  MessageBox(myTrackingDeviceSourceFactory->GetErrorMessage());
  return;
  }

//disable Buttons
m_Controls->m_StopTracking->setEnabled(true);
m_Controls->m_StartTracking->setEnabled(false);
DisableOptionsButtons();
DisableTrackingConfigurationButtons();

//initialize tracking
try
  {
  m_TrackingDeviceSource->Connect();
  m_TrackingDeviceSource->StartTracking();
  }
catch (...)
  {
  MessageBox("Error while starting the tracking device!");
  //enable Buttons
  m_Controls->m_StopTracking->setEnabled(false);
  m_Controls->m_StartTracking->setEnabled(true);
  EnableOptionsButtons();
  EnableTrackingConfigurationButtons();
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

//set configuration finished
this->m_Controls->m_configurationWidget->ConfigurationFinished();

//show tracking volume
this->OnTrackingVolumeChanged(m_Controls->m_VolumeSelectionBox->currentText());

m_tracking = true;

this->GlobalReinit();
}

void QmitkMITKIGTTrackingToolboxView::OnStopTracking()
{
if (!m_tracking) return;
m_TrackingTimer->stop();
m_TrackingDeviceSource->StopTracking();
m_TrackingDeviceSource->Disconnect();
this->m_Controls->m_configurationWidget->Reset();
m_Controls->m_TrackingControlLabel->setText("Status: stopped");
if (m_logging) StopLogging();
m_Controls->m_TrackingToolsStatusWidget->RemoveStatusLabels();
m_Controls->m_TrackingToolsStatusWidget->PreShowTools(m_toolStorage);
m_tracking = false;

//enable Buttons
m_Controls->m_StopTracking->setEnabled(false);
m_Controls->m_StartTracking->setEnabled(true);
EnableOptionsButtons();
EnableTrackingConfigurationButtons();

this->GlobalReinit();
}


void QmitkMITKIGTTrackingToolboxView::OnTrackingDeviceChanged()
{
  mitk::TrackingDeviceType Type = m_Controls->m_configurationWidget->GetTrackingDevice()->GetType();
	
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
    mitk::NDITrackingDevice::Pointer currentDevice = dynamic_cast<mitk::NDITrackingDevice*>(m_Controls->m_configurationWidget->GetTrackingDevice().GetPointer());
    currentDevice->OpenConnection();
    currentDevice->StartTracking();
    mitk::NavigationToolStorage::Pointer autoDetectedStorage = mitk::NavigationToolStorage::New(this->GetDataStorage());
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
    //save detected tools
    m_toolStorage = autoDetectedStorage;
    //update label
    QString toolLabel = QString("Loaded Tools: ") + QString::number(m_toolStorage->GetToolCount()) + " Tools (Auto Detected)";
    m_Controls->m_toolLabel->setText(toolLabel);
    //update tool preview
    m_Controls->m_TrackingToolsStatusWidget->RemoveStatusLabels();
    m_Controls->m_TrackingToolsStatusWidget->PreShowTools(m_toolStorage);
    currentDevice->StopTracking();
    currentDevice->CloseConnection();

    EnableTrackingConfigurationButtons();

    if (m_toolStorage->GetToolCount()>0)
      {
      //ask the user if he wants to save the detected tools
      QMessageBox msgBox;
      msgBox.setText("Found " + QString::number(m_toolStorage->GetToolCount()) + " tools!");
      msgBox.setInformativeText("Do you want to save this tools as tool storage, so you can load them again?");
      msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
      msgBox.setDefaultButton(QMessageBox::No);
      int ret = msgBox.exec();
      if (ret == 16384) //yes
        {
        //ask the user for a filename
        QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save File"),"",tr("*.*"));
        mitk::NavigationToolStorageSerializer::Pointer mySerializer = mitk::NavigationToolStorageSerializer::New();
        if (!mySerializer->Serialize(fileName.toStdString(),m_toolStorage)) MessageBox(mySerializer->GetErrorMessage());
        return;
        }
      else if (ret == 65536) //no
        {
        return;
        }
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
  QString filename = QFileDialog::getSaveFileName(NULL,tr("Choose Logging File"), "/", "*.*");
  this->m_Controls->m_LoggingFileName->setText(filename);
  }

void QmitkMITKIGTTrackingToolboxView::StartLogging()
  {
  if (!m_logging)
    {
    //initialize logging filter
    m_loggingFilter = mitk::NavigationDataRecorder::New();
    m_loggingFilter->SetRecordingMode(mitk::NavigationDataRecorder::NormalFile);
    if (m_Controls->m_xmlFormat->isChecked()) m_loggingFilter->SetOutputFormat(mitk::NavigationDataRecorder::xml);
    else if (m_Controls->m_csvFormat->isChecked()) m_loggingFilter->SetOutputFormat(mitk::NavigationDataRecorder::csv);
    m_loggingFilter->SetFileName(m_Controls->m_LoggingFileName->text().toStdString().c_str());
    if (m_Controls->m_LoggingLimit->isChecked()){m_loggingFilter->SetRecordCountLimit(m_Controls->m_LoggedFramesLimit->value());}

    //connect filter
    for(int i=0; i<m_ToolVisualizationFilter->GetNumberOfOutputs(); i++){m_loggingFilter->AddNavigationData(m_ToolVisualizationFilter->GetOutput(i));}

    //start filter
    m_loggingFilter->StartRecording();

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
  
  }
 
void QmitkMITKIGTTrackingToolboxView::OnAddSingleToolFinished()
  {
  m_Controls->m_TrackingToolsWidget->setCurrentIndex(0);
  if (this->m_toolStorage.IsNull()) m_toolStorage = mitk::NavigationToolStorage::New(GetDataStorage());
  m_toolStorage->AddTool(m_Controls->m_NavigationToolCreationWidget->GetCreatedTool());
  m_Controls->m_TrackingToolsStatusWidget->PreShowTools(m_toolStorage);
  QString toolLabel = QString("Loaded Tools: <manually added>");
  }
  
void QmitkMITKIGTTrackingToolboxView::OnAddSingleToolCanceled()
  {
  m_Controls->m_TrackingToolsWidget->setCurrentIndex(0);
  }


void QmitkMITKIGTTrackingToolboxView::GlobalReinit()
{
// get all nodes that have not set "includeInBoundingBox" to false
  mitk::NodePredicateNot::Pointer pred = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("includeInBoundingBox", mitk::BoolProperty::New(false)));

  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetDataStorage()->GetSubset(pred);
  // calculate bounding geometry of these nodes
  mitk::TimeSlicedGeometry::Pointer bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(rs, "visible");

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


