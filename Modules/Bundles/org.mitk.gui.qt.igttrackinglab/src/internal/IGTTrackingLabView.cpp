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
#include "IGTTrackingLabView.h"
#include "QmitkStdMultiWidget.h"

#include <QmitkNDIConfigurationWidget.h>
#include <QmitkFiducialRegistrationWidget.h>
#include <QmitkUpdateTimerWidget.h>
#include <QmitkToolTrackingStatusWidget.h>


#include <mitkCone.h>

#include <vtkConeSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkAppendPolyData.h>

// Qt
#include <QMessageBox>


const std::string IGTTrackingLabView::VIEW_ID = "org.mitk.views.igttrackinglab";

IGTTrackingLabView::IGTTrackingLabView()
: QmitkFunctionality()
,m_Source(NULL)
,m_FiducialRegistrationFilter(NULL)
,m_Visualizer(NULL)
,m_RegistrationTrackingFiducialsFilter(NULL)
,m_RegistrationTrackingFiducialsName("Tracking Fiducials")
,m_RegistrationImageFiducialsName("Image Fiducials")
{
}

IGTTrackingLabView::~IGTTrackingLabView()
{
}

void IGTTrackingLabView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  
  m_ToolBox = new QToolBox(parent);
  m_Controls.m_VBoxLayout->addWidget(m_ToolBox);


  this->CreateBundleWidgets( parent );


}


void IGTTrackingLabView::CreateBundleWidgets( QWidget* parent )
{
    // configuration widget
  m_NDIConfigWidget = new QmitkNDIConfigurationWidget(parent);
  m_NDIConfigWidget->SetToolTypes(QStringList () << "Instrument" << "Fiducial" << "Skinmarker" << "Unknown" );

  m_ToolBox->addItem(m_NDIConfigWidget, "Configuration");

  // registration widget
  m_RegistrationWidget = new QmitkFiducialRegistrationWidget(parent);
  m_RegistrationWidget->HideStaticRegistrationRadioButton(true);
  m_RegistrationWidget->HideContinousRegistrationRadioButton(true);
  //m_RegistrationWidget->HideUseICPRegistrationCheckbox(true);
  m_RegistrationWidget->SetQualityDisplayText("FRE");

  m_ToolBox->addItem(m_RegistrationWidget, "Registration");



  // tracking status
  m_ToolStatusWidget = new QmitkToolTrackingStatusWidget( parent );
 
  m_Controls.m_VBoxLayout->addWidget(m_ToolStatusWidget);

  // update timer
  m_RenderingTimerWidget = new QmitkUpdateTimerWidget( parent );
  m_RenderingTimerWidget->SetPurposeLabelText(QString("Navigation"));
  m_RenderingTimerWidget->SetTimerInterval( 50 );  // set rendering timer at 20Hz (updating every 50msec)

   m_Controls.m_VBoxLayout->addWidget(m_RenderingTimerWidget);

}


void IGTTrackingLabView::CreateConnections()
{
  connect( m_NDIConfigWidget, SIGNAL(Connected()), m_RenderingTimerWidget, SLOT(EnableWidget()) );
  connect( m_NDIConfigWidget, SIGNAL(Disconnected()), this, SLOT(OnTrackerDisconnected()) );
  connect( m_NDIConfigWidget, SIGNAL(Connected()), this, SLOT(OnSetupNavigation()) );
  connect( m_NDIConfigWidget, SIGNAL(SignalToolNameChanged(int, QString)), this, SLOT(OnChangeToolName(int, QString)) );
  connect( m_NDIConfigWidget, SIGNAL(SignalLoadTool(int, mitk::DataNode::Pointer)), this, SLOT(OnToolLoaded(int, mitk::DataNode::Pointer)) );    

  connect( m_RegistrationWidget, SIGNAL(AddedTrackingFiducial()), this, SLOT(OnAddRegistrationTrackingFiducial()) );
  connect( m_RegistrationWidget, SIGNAL(PerformFiducialRegistration()), this, SLOT(OnRegisterFiducials()) );

  connect( m_RenderingTimerWidget, SIGNAL(Started()), this, SLOT(OnStartNavigation()) );
  connect( m_RenderingTimerWidget, SIGNAL(Stopped()), this, SLOT(OnStopNavigation()) );

  
}


void IGTTrackingLabView::OnAddRegistrationTrackingFiducial()
{
  if (m_RegistrationTrackingFiducialsFilter.IsNull())
  {
    std::string message( "IGT Pipeline is not ready. Please 'Start Navigation' before adding points");
    QMessageBox::warning(NULL, "Adding Fiducials not possible", message.c_str());
    return;
  }
  if (m_RegistrationTrackingFiducialsFilter->GetInput()->IsDataValid() == false)
  {
    std::string message("instrument can currently not be tracked. Please make sure that the instrument is visible to the tracker");
    QMessageBox::warning(NULL, "Adding Fiducials not possible", message.c_str());
    return;
  }
  m_RegistrationTrackingFiducialsFilter->Update();
}

void IGTTrackingLabView::OnSetupNavigation()
{
  if(m_Source.IsNotNull())
    if(m_Source->IsTracking())
      return;

  mitk::DataStorage* ds = this->GetDefaultDataStorage();
  if(ds == NULL)
  {
    QMessageBox::warning(NULL, "IGTSurfaceTracker: Error", "can not access DataStorage. Navigation not possible");
    return;
  }

  // Building up the filter pipeline
  try
  {
    this->SetupIGTPipeline();
    
  }
  catch(std::exception& e)
  {
    QMessageBox::warning(NULL, QString("IGTSurfaceTracker: Error"), QString("Error while building the IGT-Pipeline: %1").arg(e.what()));
    this->DestroyIGTPipeline(); // destroy the pipeline if building is incomplete
    return;
  }
  catch(...)
  {
    QMessageBox::warning(NULL, QString("IGTSurfaceTracker: Error"), QString("Error while building the IGT-Pipeline"));
    this->DestroyIGTPipeline();
    return;
  }
}

void IGTTrackingLabView::SetupIGTPipeline()
{
  mitk::DataStorage* ds = this->GetDefaultDataStorage(); // check if DataStorage is available
  if(ds == NULL)
    throw std::invalid_argument("DataStorage is not available");

  mitk::TrackingDevice::Pointer tracker = m_NDIConfigWidget->GetTracker(); // get current tracker from configuration widget
  if(tracker.IsNull()) // check if tracker is valid
    throw std::invalid_argument("tracking device is NULL!");

  m_Source = mitk::TrackingDeviceSource::New(); // create new source for the IGT-Pipeline
  m_Source->SetTrackingDevice(tracker); // set the found tracker from the configuration widget to the source

  this->InitializeFilters(); // initialize all needed filters 

  for (unsigned int i=0; i < m_Source->GetNumberOfOutputs(); ++i)
  {
    m_FiducialRegistrationFilter->SetInput(i, m_Source->GetOutput(i)); // set input for registration filter
    m_Visualizer->SetInput(i, m_FiducialRegistrationFilter->GetOutput(i)); // set input for visualization filter
   }

  for(unsigned int i= 0; i < m_Visualizer->GetNumberOfOutputs(); ++i)
  {
    const char* toolName = tracker->GetTool(i)->GetToolName();

    mitk::DataNode::Pointer representation = this->CreateInstrumentVisualization(this->GetDefaultDataStorage(), toolName);
    m_Visualizer->SetRepresentationObject(i, representation->GetData());
  }

 // this->CreateInstrumentVisualization(ds, tracker);//create for each single connected ND a corresponding 3D representation
}

void IGTTrackingLabView::InitializeFilters()
{
  //1. Fiducial Registration Filters
  m_FiducialRegistrationFilter = mitk::NavigationDataLandmarkTransformFilter::New(); // filter used for initial fiducial registration

  //2. Visualization Filter
  m_Visualizer = mitk::NavigationDataObjectVisualizationFilter::New(); // filter to display NavigationData
}

void IGTTrackingLabView::OnRegisterFiducials( )
{
  /* filter pipeline can only be build, if source and visualization filters exist */
  if (m_Source.IsNull() || m_Visualizer.IsNull() || m_FiducialRegistrationFilter.IsNull())
  {
    QMessageBox::warning(NULL, "Registration not possible", "Navigation pipeline is not ready. Please (re)start the navigation");
    return;
  }
  if (m_Source->IsTracking() == false)
  {
    QMessageBox::warning(NULL, "Registration not possible", "Registration only possible if navigation is running");
    return;
  }

  /* retrieve fiducials from data storage */
  mitk::DataStorage* ds = this->GetDefaultDataStorage();
  mitk::PointSet::Pointer imageFiducials = ds->GetNamedObject<mitk::PointSet>(m_RegistrationImageFiducialsName.c_str());
  mitk::PointSet::Pointer trackerFiducials = ds->GetNamedObject<mitk::PointSet>(m_RegistrationTrackingFiducialsName.c_str());
  if (imageFiducials.IsNull() || trackerFiducials.IsNull())
  {
    QMessageBox::warning(NULL, "Registration not possible", "Fiducial data objects not found. \n"
      "Please set 3 or more fiducials in the image and with the tracking system.\n\n"
      "Registration is not possible");
    return;
  }

  unsigned int minFiducialCount = 3; // \Todo: move to view option
  if ((imageFiducials->GetSize() < minFiducialCount) || (trackerFiducials->GetSize() < minFiducialCount)
    || (imageFiducials->GetSize() != trackerFiducials->GetSize()))
  {
    QMessageBox::warning(NULL, "Registration not possible", QString("Not enough fiducial pairs found. At least %1 fiducial must "
      "exist for the image and the tracking system respectively.\n"
      "Currently, %2 fiducials exist for the image, %3 fiducials exist for the tracking system").arg(minFiducialCount).arg(imageFiducials->GetSize()).arg(trackerFiducials->GetSize()));
    return;
  }

  /* now we have two PointSets with enough points to perform a landmark based transform */
  if ( m_RegistrationWidget->UseICPIsChecked() )
    m_FiducialRegistrationFilter->UseICPInitializationOn();
  else
    m_FiducialRegistrationFilter->UseICPInitializationOff();

  m_FiducialRegistrationFilter->SetSourceLandmarks(trackerFiducials);
  m_FiducialRegistrationFilter->SetTargetLandmarks(imageFiducials);

}


void IGTTrackingLabView::OnTrackerDisconnected()
{
  m_RenderingTimerWidget->DisableWidget();
  //this->DestroyInstrumentVisualization(this->GetDefaultDataStorage(), m_NDIConfigWidget->GetTracker());
}


mitk::DataNode::Pointer IGTTrackingLabView::CreateInstrumentVisualization(mitk::DataStorage* ds, const char* toolName)
{	  
    //const char* toolName = tracker->GetTool(i)->GetToolName();
    mitk::DataNode::Pointer toolRepresentationNode;
    toolRepresentationNode = ds->GetNamedNode(toolName); // check if node with same name already exists

    if(toolRepresentationNode.IsNotNull())
      ds->Remove(toolRepresentationNode); // remove old node with same name
    
    toolRepresentationNode = this->CreateConeRepresentation( toolName );
  //  m_Visualizer->SetRepresentationObject(i, toolRepresentationNode->GetData());

    ds->Add(toolRepresentationNode); // adds node to data storage      

    return toolRepresentationNode;
}


mitk::DataNode::Pointer IGTTrackingLabView::CreateConeRepresentation( const char* label )
{

  //new data
  mitk::Cone::Pointer activeToolData = mitk::Cone::New();
  vtkConeSource* vtkData = vtkConeSource::New();

  vtkData->SetRadius(7.5);
  vtkData->SetHeight(15.0);
  vtkData->SetDirection(0.0, 0.0, 1.0);
  vtkData->SetCenter(0.0, 0.0, 0.0);
  vtkData->SetResolution(20);
  vtkData->CappingOn();
  vtkData->Update();
  activeToolData->SetVtkPolyData(vtkData->GetOutput());
  vtkData->Delete();

  //new node
  mitk::DataNode::Pointer coneNode = mitk::DataNode::New();
  coneNode->SetData(activeToolData);
  coneNode->GetPropertyList()->SetProperty("name", mitk::StringProperty::New( label ));
  coneNode->GetPropertyList()->SetProperty("layer", mitk::IntProperty::New(0));
  coneNode->GetPropertyList()->SetProperty("visible", mitk::BoolProperty::New(true));
  coneNode->SetColor(1.0,0.0,0.0);
  coneNode->SetOpacity(0.85);
  coneNode->Modified();

  return coneNode;
}

void IGTTrackingLabView::DestroyIGTPipeline()
{
  if(m_Source.IsNotNull())
  {
    m_Source->StopTracking();
    m_Source->Disconnect();
    m_Source = NULL;
  }
}

void IGTTrackingLabView::OnChangeToolName(int index, QString name)
{
    if(m_Source.IsNull())
      return;
    
    mitk::DataStorage* ds = this->GetDefaultDataStorage();
    if(ds == NULL)
    {
      QMessageBox::warning(NULL,"DataStorage Access Error", "Could not access DataStorage. Tool Name can not be changed!");
      return;
    }
 
    mitk::NavigationData::Pointer tempND = m_Source->GetOutput(index);
    if(tempND.IsNull())
      return;
    
    const char* oldName = tempND->GetName();
    
    mitk::DataNode::Pointer tempNode = ds->GetNamedNode(oldName);
    
    if(tempNode.IsNotNull())
    {
      tempNode->SetName(name.toStdString().c_str());
      tempND->SetName(name.toStdString().c_str());
    } 
    else
      QMessageBox::warning(NULL, "Rename Tool Error", "Couldn't find the corresponding tool for changing it's name!");  
}

void IGTTrackingLabView::OnToolLoaded(int index, mitk::DataNode::Pointer toolNode)
{
  if(m_Source.IsNull() || m_Visualizer.IsNull())
    return;

  mitk::DataStorage* ds = this->GetDefaultDataStorage();
  if(ds == NULL)
  {
    QMessageBox::warning(NULL,"DataStorage Access Error", "Could not access DataStorage. Loaded tool representation can not be shown!");
    return;
  }

  mitk::NavigationData::Pointer tempND = m_Source->GetOutput(index);
  if(tempND.IsNull())
    return;

  // try to find DataNode for tool in DataStorage
  const char* toolName = tempND->GetName();
  mitk::DataNode::Pointer tempNode = ds->GetNamedNode(toolName);

  if(tempNode.IsNull())
  {
    tempNode = mitk::DataNode::New();  // create new node, if none was found
    ds->Add(tempNode);
  }

  tempNode->SetData(toolNode->GetData());
  tempNode->SetName(toolNode->GetName()); 
  
  m_Visualizer->SetRepresentationObject(index, tempNode->GetData());
  m_Visualizer->Update();
  //this->GlobalReinit();
}

void IGTTrackingLabView::OnStartNavigation()
{
  if(m_Source.IsNull())
  {
    QMessageBox::warning(NULL, "IGTTrackingLab: Error", "can not access tracking source. Navigation not possible");
    return;
  }

  if(!m_Source->IsTracking())
  {
    m_Source->StartTracking();

    try
    {
      m_RenderingTimerWidget->GetTimerInterval();
      this->StartContinuousUpdate(); // start tracker with set interval

      for(unsigned int i = 0; i < m_Source->GetNumberOfOutputs(); i++)  // add navigation data to bundle widgets
      {
        m_ToolStatusWidget->AddNavigationData(dynamic_cast<mitk::NavigationData*>(m_Source->GetOutputs().at(i).GetPointer()));
      }

      m_ToolStatusWidget->ShowStatusLabels(); // show status for every tool if ND is valid or not
      //m_IGTPlayerWidget->setEnabled(true);
    }
    catch(...)
    {
      //m_IGTPlayerWidget->setDisabled(true);
      this->StopContinuousUpdate();
      this->DestroyIGTPipeline();
      return;
    }
    
   
  }
}


void IGTTrackingLabView::StopContinuousUpdate()
{
  if (this->m_RenderingTimerWidget->GetUpdateTimer() != NULL)
  {
    m_RenderingTimerWidget->StopTimer();
    disconnect( (QTimer*) m_RenderingTimerWidget->GetUpdateTimer(), SIGNAL(timeout()), this, SLOT(RenderScene()) ); // disconnect timer from RenderScene() method
  }
}

void IGTTrackingLabView::RenderScene( )
{
  try
  {
    if (m_Visualizer.IsNull() || this->GetActiveStdMultiWidget() == NULL)
      return;
    try
    {
      if(m_Source.IsNotNull() && m_Source->IsTracking())
        m_ToolStatusWidget->Refresh();

      m_Visualizer->Update();
    }
    catch(std::exception& e) 
    {
      MITK_WARN << "Exception during QmitkIGTTrackingLab::RenderScene():" << e.what() << "\n";
    }

    //update all Widgets
    mitk::RenderingManager::GetInstance()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
  }
  catch (std::exception& e)
  {
    MITK_WARN << "RenderAll exception: " << e.what() << "\n";
  }
  catch (...)
  {
    MITK_WARN << "RenderAll unknown exception\n";
  }
}

void IGTTrackingLabView::StartContinuousUpdate( )
{
  if (m_Source.IsNull() || m_Visualizer.IsNull() )
    throw std::invalid_argument("Pipeline is not set up correctly");

  if (m_RenderingTimerWidget->GetUpdateTimer() == NULL)
    return;

  else
  {
    connect( (QTimer*) m_RenderingTimerWidget->GetUpdateTimer(), SIGNAL(timeout()), this, SLOT(RenderScene()) ); // connect update timer to RenderScene() method
  }
}



void IGTTrackingLabView::OnStopNavigation()
{
  if(m_Source.IsNull())
  {
    QMessageBox::warning(NULL, "IGTSurfaceTracker: Error", "can not access tracking source. Navigation not possible");
    return;
  }
  if(m_Source->IsTracking())
  {
    m_Source->StopTracking();
    this->StopContinuousUpdate();
    m_ToolStatusWidget->RemoveStatusLabels();
  }
}















//void IGTTrackingLabView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
//{ 
//  // iterate all selected objects, adjust warning visibility
//  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin();
//    it != nodes.end();
//    ++it )
//  {
//    mitk::DataNode::Pointer node = *it;
//
//    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
//    {
//      m_Controls.labelWarning->setVisible( false );
//      m_Controls.buttonPerformImageProcessing->setEnabled( true );
//      return;
//    }
//  }
//
//  m_Controls.labelWarning->setVisible( true );
//  m_Controls.buttonPerformImageProcessing->setEnabled( false );
//}


