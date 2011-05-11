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
#include <mitkTrackingVolume.h>



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

    //initialize widgets
    m_Controls->m_configurationWidget->EnableAdvancedUserControl(false);
    m_Controls->m_TrackingToolsStatusWidget->SetShowPositions(true);
    m_Controls->m_TrackingToolsStatusWidget->SetTextAlignment(Qt::AlignLeft);

    //initialize tracking volume node
    TrackingVolumeNode = mitk::DataNode::New();
    TrackingVolumeNode->SetName("TrackingVolume");
    this->GetDataStorage()->Add(TrackingVolumeNode);

    //initialize buttons
    m_Controls->m_StopTracking->setEnabled(false);
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
  
  //initialize tool storage
  m_toolStorage = mitk::NavigationToolStorage::New();
  
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
mitk::TrackingDeviceSourceConfigurator::Pointer myTrackingDeviceSourceFactory = mitk::TrackingDeviceSourceConfigurator::New(this->m_toolStorage,this->m_Controls->m_configurationWidget->GetTrackingDevice());
m_TrackingDeviceSource = myTrackingDeviceSourceFactory->CreateTrackingDeviceSource(this->m_ToolVisualizationFilter);
if (m_TrackingDeviceSource.IsNull()) 
  {
  MessageBox(myTrackingDeviceSourceFactory->GetErrorMessage());
  return;
  }

//initialize tracking
try 
  {
  m_TrackingDeviceSource->Connect();
  m_TrackingDeviceSource->StartTracking();
  }
catch (...)
  {
  MessageBox("Error while starting the tracking device!");
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

//disable loading new tools
this->m_Controls->m_LoadTools->setEnabled(false);

//set configuration finished
this->m_Controls->m_configurationWidget->ConfigurationFinished();

//show tracking volume
if (m_Controls->m_ShowTrackingVolume->isChecked())
  {
  mitk::TrackingVolume::Pointer volumeSurface = mitk::TrackingVolume::New();
  volumeSurface->SetTrackingDeviceType(m_TrackingDeviceSource->GetTrackingDevice()->GetType());
  TrackingVolumeNode->SetData(volumeSurface);
  TrackingVolumeNode->SetOpacity(0.25);
  mitk::Color red;
  red.SetRed(1);
  TrackingVolumeNode->SetColor(red);
  }

m_tracking = true;
m_Controls->m_StopTracking->setEnabled(true);
m_Controls->m_StartTracking->setEnabled(false);

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
this->m_Controls->m_LoadTools->setEnabled(true);
m_Controls->m_TrackingToolsStatusWidget->RemoveStatusLabels();
m_Controls->m_TrackingToolsStatusWidget->PreShowTools(m_toolStorage);
TrackingVolumeNode->SetData(NULL);
m_tracking = false;
m_Controls->m_StopTracking->setEnabled(false);
m_Controls->m_StartTracking->setEnabled(true);
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
    if((m_loggedFrames>1)&&(!m_loggingFilter->GetRecording())) 
      {
      m_Controls->m_LoggingLabel->setText("Logging OFF");
      m_logging = false;
      }
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
  }

void QmitkMITKIGTTrackingToolboxView::StopLogging()
  {
  //update label
  this->m_Controls->m_LoggingLabel->setText("Logging OFF");

  m_loggingFilter->StopRecording();
  m_logging = false;
  }



