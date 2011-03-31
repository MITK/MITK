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



const std::string QmitkMITKIGTTrackingToolboxView::VIEW_ID = "org.mitk.views.mitkigttrackingtoolbox";

QmitkMITKIGTTrackingToolboxView::QmitkMITKIGTTrackingToolboxView()
: QmitkFunctionality()
, m_Controls( 0 )
, m_MultiWidget( NULL )
{
  m_TrackingTimer = new QTimer(this);	
  m_tracking = false;
  m_logging = false;
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
    connect( m_Controls->m_EnableLogging, SIGNAL(clicked()), this, SLOT(OnEnableLoggingClicked()));
    connect( m_Controls->m_ChooseFile, SIGNAL(clicked()), this, SLOT(OnChooseFileClicked()));

    //initialize widgets
    m_Controls->m_configurationWidget->EnableAdvancedUserControl(false);
    m_Controls->m_TrackingToolsStatusWidget->SetShowPositions(true);
    m_Controls->m_TrackingToolsStatusWidget->SetTextAlignment(Qt::AlignLeft);
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


//set configuration finished
this->m_Controls->m_configurationWidget->ConfigurationFinished();

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
m_TrackingTimer->start(100);
m_Controls->m_TrackingControlLabel->setText("Status: tracking");

//start logging if logging is on
if (this->m_Controls->m_EnableLogging->isChecked()) StartLogging();

//connect the tool visualization widget
for(int i=0; i<m_TrackingDeviceSource->GetNumberOfOutputs(); i++) 
  {
    m_Controls->m_TrackingToolsStatusWidget->AddNavigationData(m_TrackingDeviceSource->GetOutput(i));
  }
m_Controls->m_TrackingToolsStatusWidget->ShowStatusLabels();

//disable loading new tools
this->m_Controls->m_LoadTools->setEnabled(false);

m_tracking = true;
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
  //std::cout << "Position" << m_ToolVisualizationFilter->GetOutput(0)->GetPosition() << std::endl;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  if (m_logging) this->m_loggingFilter->Update();
  m_Controls->m_TrackingToolsStatusWidget->Refresh();
  }

void QmitkMITKIGTTrackingToolboxView::OnEnableLoggingClicked()
  {
  if (this->m_tracking && this->m_Controls->m_EnableLogging->isChecked() && !this->m_logging) StartLogging();
  else if (!this->m_Controls->m_EnableLogging->isChecked() && this->m_logging) StopLogging();
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
  
  //connect filter
  for(int i=0; i<m_ToolVisualizationFilter->GetNumberOfOutputs(); i++){m_loggingFilter->AddNavigationData(m_ToolVisualizationFilter->GetOutput(i));}
  
  m_loggingFilter->StartRecording();
  m_logging = true;
  }

void QmitkMITKIGTTrackingToolboxView::StopLogging()
  {
  m_loggingFilter->StopRecording();
  m_logging = false;
  }



