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
#include "QmitkIGTTrackingLabView.h"
#include "QmitkStdMultiWidget.h"

#include <QmitkNDIConfigurationWidget.h>
#include <QmitkFiducialRegistrationWidget.h>
#include <QmitkUpdateTimerWidget.h>
#include <QmitkToolSelectionWidget.h>
#include <QmitkToolTrackingStatusWidget.h>


#include <mitkCone.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

#include <vtkConeSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkAppendPolyData.h>

// Qt
#include <QMessageBox>
#include <QIcon>


const std::string QmitkIGTTrackingLabView::VIEW_ID = "org.mitk.views.igttrackinglab";

QmitkIGTTrackingLabView::QmitkIGTTrackingLabView()
: QmitkFunctionality()
,m_Source(NULL)
,m_FiducialRegistrationFilter(NULL)
,m_PermanentRegistrationFilter(NULL)
,m_Visualizer(NULL)
,m_VirtualView(NULL)
,m_PSRecordingPointSet(NULL)
,m_RegistrationTrackingFiducialsName("Tracking Fiducials")
,m_RegistrationImageFiducialsName("Image Fiducials")
,m_PointSetRecordingDataNodeName("Recorded Points")
,m_PointSetRecording(false)
,m_ImageFiducialsDataNode(NULL)
,m_TrackerFiducialsDataNode(NULL)
,m_PermanentRegistrationSourcePoints(NULL)
{

  //[-1;0;0] for WOLF_6D bronchoscope       
  m_DirectionOfProjectionVector[0]=0;
  m_DirectionOfProjectionVector[1]=0;
  m_DirectionOfProjectionVector[2]=-1;}

QmitkIGTTrackingLabView::~QmitkIGTTrackingLabView()
{
}

void QmitkIGTTrackingLabView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  
  m_ToolBox = new QToolBox(parent);
  m_Controls.m_VBoxLayout->addWidget(m_ToolBox);


  this->CreateBundleWidgets( parent );
  this->CreateConnections();
}


void QmitkIGTTrackingLabView::CreateBundleWidgets( QWidget* parent )
{
    // configuration widget
  m_NDIConfigWidget = new QmitkNDIConfigurationWidget(parent);
  m_NDIConfigWidget->SetToolTypes(QStringList () << "Instrument" << "Fiducial" << "Skinmarker" << "Unknown" );

  m_ToolBox->addItem(m_NDIConfigWidget, "Configuration");


  // registration widget
  m_RegistrationWidget = new QmitkFiducialRegistrationWidget(parent);
  m_RegistrationWidget->HideStaticRegistrationRadioButton(true);
  m_RegistrationWidget->HideContinousRegistrationRadioButton(true);
  m_RegistrationWidget->HideUseICPRegistrationCheckbox(true);

  m_ToolBox->addItem(m_RegistrationWidget, "Initial Registration");

  // permanent registration widget
  m_PermanentRegistrationToolSelectionWidget = new QmitkToolSelectionWidget(parent);
  m_PermanentRegistrationToolSelectionWidget->SetCheckboxtText("Use this tool for permanent registration");

  m_ToolBox->addItem(m_PermanentRegistrationToolSelectionWidget, "Permanent Registration");

  // pointset recording
  m_ToolBox->addItem(this->CreatePointSetRecordingWidget(parent), "PointSet Recording");

  // virtual view
  m_VirtualViewToolSelectionWidget = new QmitkToolSelectionWidget(parent);
  m_VirtualViewToolSelectionWidget->SetCheckboxtText("Enable Virtual Camera");

  m_ToolBox->addItem(m_VirtualViewToolSelectionWidget, "Virtual Camera");

  // tracking status
  m_ToolStatusWidget = new QmitkToolTrackingStatusWidget( parent );
  m_Controls.m_VBoxLayout->addWidget(m_ToolStatusWidget);

  // update timer
  m_RenderingTimerWidget = new QmitkUpdateTimerWidget( parent );
  m_RenderingTimerWidget->SetPurposeLabelText(QString("Navigation"));
  m_RenderingTimerWidget->SetTimerInterval( 50 );  // set rendering timer at 20Hz (updating every 50msec)

  m_Controls.m_VBoxLayout->addWidget(m_RenderingTimerWidget);
}


void QmitkIGTTrackingLabView::CreateConnections()
{
  connect( m_ToolBox, SIGNAL(currentChanged(int)), this, SLOT(OnToolBoxCurrentChanged(int)) );

  //connect( m_NDIConfigWidget, SIGNAL(Connected()), m_RenderingTimerWidget, SLOT(EnableWidget()) );
  connect( m_NDIConfigWidget, SIGNAL(Disconnected()), this, SLOT(OnTrackerDisconnected()) );
  connect( m_NDIConfigWidget, SIGNAL(Connected()), this, SLOT(OnSetupNavigation()) );
  connect( m_NDIConfigWidget, SIGNAL(SignalToolNameChanged(int, QString)), this, SLOT(OnChangeToolName(int, QString)) );
  connect( m_NDIConfigWidget, SIGNAL(SignalLoadTool(int, mitk::DataNode::Pointer)), this, SLOT(OnToolLoaded(int, mitk::DataNode::Pointer)) );    
  connect( m_NDIConfigWidget, SIGNAL(ToolsAdded(QStringList)), this, SLOT(OnToolsAdded(QStringList)) );
  connect( m_NDIConfigWidget, SIGNAL(RepresentationChanged( int ,mitk::Surface::Pointer )), this, SLOT(ChangeToolRepresentation( int, mitk::Surface::Pointer )));

  connect( m_RegistrationWidget, SIGNAL(AddedTrackingFiducial()), this, SLOT(OnAddRegistrationTrackingFiducial()) );
  connect( m_RegistrationWidget, SIGNAL(PerformFiducialRegistration()), this, SLOT(OnRegisterFiducials()) );

  connect( m_RenderingTimerWidget, SIGNAL(Started()), this, SLOT(OnStartNavigation()) );
  connect( m_RenderingTimerWidget, SIGNAL(Stopped()), this, SLOT(OnStopNavigation()) );

  connect( m_VirtualViewToolSelectionWidget, SIGNAL(SignalUseTool(int, bool)), this, SLOT(OnVirtualCamera(int, bool)));
  
  connect( m_PermanentRegistrationToolSelectionWidget, SIGNAL(SignalUseTool(int, bool)), this, SLOT(OnPermanentRegistration(int, bool)) );

}


void QmitkIGTTrackingLabView::OnAddRegistrationTrackingFiducial()
{

   mitk::DataStorage* ds = this->GetDefaultDataStorage(); // check if DataStorage available
  if(ds == NULL)
    throw std::invalid_argument("DataStorage is not available");

  if (m_FiducialRegistrationFilter.IsNull())
  {
    std::string message( "IGT Pipeline is not ready. Please 'Start Navigation' before adding points");
    QMessageBox::warning(NULL, "Adding Fiducials not possible", message.c_str());
    return;
  }
  
  if (m_FiducialRegistrationFilter->GetNumberOfOutputs() < 1 || m_FiducialRegistrationFilter->GetNumberOfInputs() < 1)
  {
    std::string message("There are no tracking instruments! Please add an instrument first!");
    QMessageBox::warning(NULL, "Adding Fiducials not possible", message.c_str());
    return;
  }

  if (m_FiducialRegistrationFilter->GetInput()->IsDataValid() == false)
  {
    std::string message("instrument can currently not be tracked. Please make sure that the instrument is visible to the tracker");
    QMessageBox::warning(NULL, "Adding Fiducials not possible", message.c_str());
    return;
  }

  mitk::NavigationData::Pointer nd = m_Source->GetOutput(0);
  
  if( nd.IsNull() || !nd->IsDataValid())
    QMessageBox::warning( 0, "Invalid tracking data", "Navigation data is not available or invalid!", QMessageBox::Ok );

  // in case the tracker fiducials datanode has been renamed or removed
  //if(trackerFiducialsPS.IsNull())
  //{
  //  mitk::DataNode::Pointer trackerFiducialsDN = mitk::DataNode::New();
  //  trackerFiducialsDN->SetName(m_RegistrationTrackingFiducialsName);
  //  trackerFiducialsPS = mitk::PointSet::New();
  //  trackerFiducialsDN->SetData(trackerFiducialsPS);
  //  m_RegistrationWidget->SetTrackerFiducialsNode(trackerFiducialsDN);
  //}

  

  if(m_TrackerFiducialsDataNode.IsNotNull() && m_TrackerFiducialsDataNode->GetData() != NULL)
  {
    mitk::PointSet::Pointer ps = dynamic_cast<mitk::PointSet*>(m_TrackerFiducialsDataNode->GetData());
    ps->InsertPoint(ps->GetSize(), nd->GetPosition());
  }
  else
    QMessageBox::warning(NULL, "IGTSurfaceTracker: Error", "Can not access Tracker Fiducials. Adding fiducial not possible!");
   

}

void QmitkIGTTrackingLabView::OnSetupNavigation()
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

void QmitkIGTTrackingLabView::SetupIGTPipeline()
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

  if(m_NDIConfigWidget->GetTracker()->GetType() == mitk::NDIAurora)
  {

    for (unsigned int i=0; i < m_Source->GetNumberOfOutputs(); ++i)
    {
      m_FiducialRegistrationFilter->SetInput(i, m_Source->GetOutput(i)); // set input for registration filter
      m_Visualizer->SetInput(i, m_FiducialRegistrationFilter->GetOutput(i)); // set input for visualization filter
    }

    for(unsigned int i= 0; i < m_Visualizer->GetNumberOfOutputs(); ++i)
    {
      const char* toolName = tracker->GetTool(i)->GetToolName();

      mitk::DataNode::Pointer representation = this->CreateInstrumentVisualization(this->GetDefaultDataStorage(), toolName);
      m_PSRecToolSelectionComboBox->addItem(QString(toolName));

      m_PermanentRegistrationToolSelectionWidget->AddToolName(QString(toolName));
      m_VirtualViewToolSelectionWidget->AddToolName(QString(toolName));

      m_Visualizer->SetRepresentationObject(i, representation->GetData());

    }

    if(m_Source->GetTrackingDevice()->GetToolCount() > 0)
      m_RenderingTimerWidget->setEnabled(true);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_ALL);
    this->GlobalReinit();
  }

  // this->CreateInstrumentVisualization(ds, tracker);//create for each single connected ND a corresponding 3D representation
}

void QmitkIGTTrackingLabView::InitializeFilters()
{
  //1. Fiducial Registration Filters
  m_FiducialRegistrationFilter = mitk::NavigationDataLandmarkTransformFilter::New(); // filter used for initial fiducial registration

  //2. Visualization Filter
  m_Visualizer = mitk::NavigationDataObjectVisualizationFilter::New(); // filter to display NavigationData

  m_PermanentRegistrationFilter = mitk::NavigationDataLandmarkTransformFilter::New();

  //3. Virtual Camera
  m_VirtualView = mitk::CameraVisualization::New(); // filter to update the vtk camera according to the reference navigation data
  m_VirtualView->SetRenderer(mitk::BaseRenderer::GetInstance(this->GetActiveStdMultiWidget()->mitkWidget4->GetRenderWindow()));

  mitk::Vector3D viewUpInToolCoordinatesVector;
  viewUpInToolCoordinatesVector[0]=1;
  viewUpInToolCoordinatesVector[1]=0;
  viewUpInToolCoordinatesVector[2]=0;

  m_VirtualView->SetDirectionOfProjectionInToolCoordinates(m_DirectionOfProjectionVector);
  m_VirtualView->SetFocalLength(5000.0); 
  m_VirtualView->SetViewUpInToolCoordinates(viewUpInToolCoordinatesVector);
}

void QmitkIGTTrackingLabView::OnRegisterFiducials( )
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


  mitk::PointSet::Pointer imageFiducials = dynamic_cast<mitk::PointSet*>(m_ImageFiducialsDataNode->GetData());
  mitk::PointSet::Pointer trackerFiducials = dynamic_cast<mitk::PointSet*>(m_TrackerFiducialsDataNode->GetData());
 
  //mitk::PointSet::Pointer imageFiducials = ds->GetNamedObject<mitk::PointSet>(m_RegistrationImageFiducialsName.c_str());
  //mitk::PointSet::Pointer trackerFiducials = ds->GetNamedObject<mitk::PointSet>(m_RegistrationTrackingFiducialsName.c_str());
  if (imageFiducials.IsNull() || trackerFiducials.IsNull())
  {
    QMessageBox::warning(NULL, "Registration not possible", "Fiducial data objects not found. \n"
      "Please set 3 or more fiducials in the image and with the tracking system.\n\n"
      "Registration is not possible");
    return;
  }

  unsigned int minFiducialCount = 3; // \Todo: move to view option
  if ((imageFiducials->GetSize() < minFiducialCount) || (trackerFiducials->GetSize() < minFiducialCount) || (imageFiducials->GetSize() != trackerFiducials->GetSize()))
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


  if (m_FiducialRegistrationFilter.IsNotNull() && m_FiducialRegistrationFilter->IsInitialized()) // update registration quality display
    {
      QString registrationQuality = QString("%0: FRE is %1mm (Std.Dev. %2), \n"
        "RMS error is %3mm,\n"
        "Minimum registration error (best fitting landmark) is  %4mm,\n"
        "Maximum registration error (worst fitting landmark) is %5mm.")
        .arg("Fiducial Registration")
        .arg(m_FiducialRegistrationFilter->GetFRE(), 3, 'f', 3)
        .arg(m_FiducialRegistrationFilter->GetFREStdDev(), 3, 'f', 3)
        .arg(m_FiducialRegistrationFilter->GetRMSError(), 3, 'f', 3)
        .arg(m_FiducialRegistrationFilter->GetMinError(), 3, 'f', 3)
        .arg(m_FiducialRegistrationFilter->GetMaxError(), 3, 'f', 3);
      m_RegistrationWidget->SetQualityDisplayText(registrationQuality);
    }

  //trackerFiducials->Clear();
  //this->GlobalReinit();
}


void QmitkIGTTrackingLabView::OnTrackerDisconnected()
{
  m_RenderingTimerWidget->DisableWidget();
  this->DestroyInstrumentVisualization(this->GetDefaultDataStorage(), m_NDIConfigWidget->GetTracker());
}


mitk::DataNode::Pointer QmitkIGTTrackingLabView::CreateInstrumentVisualization(mitk::DataStorage* ds, const char* toolName)
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


mitk::DataNode::Pointer QmitkIGTTrackingLabView::CreateConeRepresentation( const char* label )
{

  //new data
  mitk::Cone::Pointer activeToolData = mitk::Cone::New();
  vtkConeSource* vtkData = vtkConeSource::New();

  vtkData->SetRadius(7.5);
  vtkData->SetHeight(15.0);
  vtkData->SetDirection(m_DirectionOfProjectionVector[0],m_DirectionOfProjectionVector[1],m_DirectionOfProjectionVector[2]);
  vtkData->SetCenter(0.0, 0.0, 7.5);
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

void QmitkIGTTrackingLabView::DestroyIGTPipeline()
{
  if(m_Source.IsNotNull())
  {
    m_Source->StopTracking();
    m_Source->Disconnect();
    m_Source = NULL;
  }
  m_FiducialRegistrationFilter = NULL;
  m_PermanentRegistrationFilter = NULL;
  m_Visualizer = NULL;
  m_VirtualView = NULL;
}

void QmitkIGTTrackingLabView::OnChangeToolName(int index, QString name)
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

void QmitkIGTTrackingLabView::OnToolLoaded(int index, mitk::DataNode::Pointer toolNode)
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

  m_PSRecToolSelectionComboBox->setItemText(index,toolNode->GetName().c_str());

  m_VirtualViewToolSelectionWidget->ChangeToolName(index, QString(toolNode->GetName().c_str()));
  m_PermanentRegistrationToolSelectionWidget->ChangeToolName(index, QString(toolNode->GetName().c_str()));

  m_Visualizer->SetRepresentationObject(index, tempNode->GetData());
  m_Visualizer->Update();
 
  tempNode->Modified();
  this->GlobalReinit();
}

void QmitkIGTTrackingLabView::OnStartNavigation()
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
    
    m_NDIConfigWidget->EnableAddToolsButton(false);
   
  }
}


void QmitkIGTTrackingLabView::StopContinuousUpdate()
{
  if (this->m_RenderingTimerWidget->GetUpdateTimer() != NULL)
  {
    m_RenderingTimerWidget->StopTimer();
    disconnect( (QTimer*) m_RenderingTimerWidget->GetUpdateTimer(), SIGNAL(timeout()), this, SLOT(RenderScene()) ); // disconnect timer from RenderScene() method
  }

  if(m_PointSetRecordPushButton)
    m_PointSetRecordPushButton->setDisabled(true);
}

void QmitkIGTTrackingLabView::RenderScene( )
{
  try
  {
    if (m_Visualizer.IsNull() || this->GetActiveStdMultiWidget() == NULL)
      return;
    try
    {
      if(m_Source.IsNotNull() && m_Source->IsTracking())
       m_ToolStatusWidget->Refresh();

      
      
      if(m_VirtualViewToolSelectionWidget->IsSelectedToolActivated())
      {
        m_VirtualView->Update();
        mitk::Point3D p = m_Visualizer->GetOutput(m_VirtualViewToolSelectionWidget->GetCurrentSelectedIndex())->GetPosition();
        this->GetActiveStdMultiWidget()->MoveCrossToPosition(p);
      }

      if(m_PermanentRegistrationToolSelectionWidget->IsSelectedToolActivated() && m_PermanentRegistrationToolSelectionWidget->GetCurrentSelectedIndex() >= 0 )
      {
        mitk::NavigationData::Pointer permRegTool = m_Source->GetOutput((unsigned int) m_PermanentRegistrationToolSelectionWidget->GetCurrentSelectedIndex());
        
        m_PermanentRegistrationFilter->SetSourceLandmarks(this->GetVirtualPointSetFromPosition(permRegTool));
      }

      if(m_PointSetRecording && m_PSRecordingPointSet.IsNotNull())
      {
        int size = m_PSRecordingPointSet->GetSize();
        mitk::NavigationData::Pointer nd= m_Visualizer->GetOutput(m_PSRecToolSelectionComboBox->currentIndex());
        
        if(size > 0)
        {
          mitk::Point3D p = m_PSRecordingPointSet->GetPoint(size-1);
          if(p.EuclideanDistanceTo(nd->GetPosition()) > (double) m_PSRecordingSpinBox->value())
            m_PSRecordingPointSet->InsertPoint(size, nd->GetPosition());
        }
        else
          m_PSRecordingPointSet->InsertPoint(size, nd->GetPosition());
      }
    }
    catch(std::exception& e) 
    {
      MITK_WARN << "Exception during QmitkIGTTrackingLab::RenderScene():" << e.what() << "\n";
    }

    //if(m_VirtualViewCheckBox->isChecked())
    //  mitk::RenderingManager::GetInstance()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_ALL);
    ////update all Widgets
    //else

    m_Visualizer->Update();

      mitk::RenderingManager::GetInstance()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_ALL);


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

void QmitkIGTTrackingLabView::StartContinuousUpdate( )
{
  if (m_Source.IsNull() || m_Visualizer.IsNull() )
    throw std::invalid_argument("Pipeline is not set up correctly");

  if (m_RenderingTimerWidget->GetUpdateTimer() == NULL)
    return;

  else
  {
    connect( (QTimer*) m_RenderingTimerWidget->GetUpdateTimer(), SIGNAL(timeout()), this, SLOT(RenderScene()) ); // connect update timer to RenderScene() method
  }

  if(m_PointSetRecordPushButton)
    m_PointSetRecordPushButton->setEnabled(true);
}



void QmitkIGTTrackingLabView::OnStopNavigation()
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

    m_NDIConfigWidget->EnableAddToolsButton(true);
  }
}


void QmitkIGTTrackingLabView::OnToolsAdded(QStringList toolsList)
{

  if(m_Source.IsNull() || m_FiducialRegistrationFilter.IsNull() || m_Visualizer.IsNull())
    return;

  m_Source->UpdateOutputInformation();

  unsigned int nrOfOldOutputs = m_Visualizer->GetNumberOfOutputs();

  for(unsigned int i = nrOfOldOutputs; i < m_Source->GetNumberOfOutputs(); ++i)
  {
    m_FiducialRegistrationFilter->SetInput(i, m_Source->GetOutput(i));
    m_Visualizer->SetInput(i, m_FiducialRegistrationFilter->GetOutput(i));
  }

  for(unsigned int j = nrOfOldOutputs; j < m_Visualizer->GetNumberOfOutputs(); ++j)
  {
    mitk::DataNode::Pointer representation = this->CreateInstrumentVisualization(this->GetDefaultDataStorage(), m_Source->GetTrackingDevice()->GetTool(j)->GetToolName());
    
    m_PSRecToolSelectionComboBox->addItem(QString(m_Source->GetTrackingDevice()->GetTool(j)->GetToolName()));
    
    m_PermanentRegistrationToolSelectionWidget->AddToolName(QString(m_Source->GetTrackingDevice()->GetTool(j)->GetToolName()));
    m_VirtualViewToolSelectionWidget->AddToolName(QString(m_Source->GetTrackingDevice()->GetTool(j)->GetToolName()));

    m_Visualizer->SetRepresentationObject(j, representation->GetData());
  }

  if(m_Source->GetTrackingDevice()->GetToolCount() > 0)
    m_RenderingTimerWidget->setEnabled(true);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_ALL);
  this->GlobalReinit();

  //mitk::RenderingManager::GetInstance()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_ALL);
}

void QmitkIGTTrackingLabView::InitializeRegistration()
{
  mitk::DataStorage* ds = this->GetDefaultDataStorage();
  if( ds == NULL )
    return;

    
  m_RegistrationWidget->SetMultiWidget(this->GetActiveStdMultiWidget()); // passing multiwidget to pointsetwidget

  if(m_ImageFiducialsDataNode.IsNull())
  {
    m_ImageFiducialsDataNode = mitk::DataNode::New();
    mitk::PointSet::Pointer ifPS = mitk::PointSet::New();
   
    m_ImageFiducialsDataNode->SetData(ifPS);
    
    mitk::Color color;
    color.Set(1.0f, 0.0f, 0.0f);
    m_ImageFiducialsDataNode->SetName(m_RegistrationImageFiducialsName);
    m_ImageFiducialsDataNode->SetColor(color);
    m_ImageFiducialsDataNode->SetBoolProperty( "updateDataOnRender", false );
   
    ds->Add(m_ImageFiducialsDataNode);
  }
  m_RegistrationWidget->SetMultiWidget(this->GetActiveStdMultiWidget());
  m_RegistrationWidget->SetImageFiducialsNode(m_ImageFiducialsDataNode);
  
  if(m_TrackerFiducialsDataNode.IsNull())
  {
    m_TrackerFiducialsDataNode = mitk::DataNode::New();
    mitk::PointSet::Pointer tfPS = mitk::PointSet::New();
    m_TrackerFiducialsDataNode->SetData(tfPS);
    
    mitk::Color color;
    color.Set(0.0f, 1.0f, 0.0f);
    m_TrackerFiducialsDataNode->SetName(m_RegistrationTrackingFiducialsName);
    m_TrackerFiducialsDataNode->SetColor(color);
    m_TrackerFiducialsDataNode->SetBoolProperty( "updateDataOnRender", false );
   
    ds->Add(m_TrackerFiducialsDataNode);
  }
  
  m_RegistrationWidget->SetTrackerFiducialsNode(m_TrackerFiducialsDataNode);
}


void QmitkIGTTrackingLabView::OnToolBoxCurrentChanged(const int index)
{
  switch (index)
  {
  case RegistrationWidget:
    this->InitializeRegistration();
    break;

  default:
    break;
  }
}


mitk::DataNode::Pointer QmitkIGTTrackingLabView::CreateRegistrationFiducialsNode( const std::string& label, const mitk::Color& color)
{
  mitk::DataNode::Pointer fiducialsNode = mitk::DataNode::New();
  mitk::PointSet::Pointer fiducialsPointSet = mitk::PointSet::New();

  fiducialsNode->SetData(fiducialsPointSet);
  fiducialsNode->SetName( label );
  fiducialsNode->SetColor( color );
  fiducialsNode->SetBoolProperty( "updateDataOnRender", false );

  return fiducialsNode;
}


void QmitkIGTTrackingLabView::ChangeToolRepresentation( int toolID , mitk::Surface::Pointer surface )
{
  mitk::DataStorage* ds = this->GetDefaultDataStorage();
  if(ds == NULL)
  {
    QMessageBox::warning(NULL, "IGTSurfaceTracker: Error", "Can not access DataStorage. Changing tool representation not possible!");
    return;
  }

  mitk::TrackingDevice::Pointer tracker = m_NDIConfigWidget->GetTracker();
  if(tracker.IsNull())
  {
    QMessageBox::warning(NULL, "IGTSurfaceTracker: Error", "Can not access Tracker. Changing tool representation not possible!");
    return;
  }

  try
  {
    const char* name = tracker->GetTool(toolID)->GetToolName();   // get tool name by id

    mitk::DataNode::Pointer toolNode = ds->GetNamedNode(name);

    if(toolNode.IsNull())
      return;

    toolNode->SetData(surface); // change surface representation of node
    toolNode->SetColor(0.45,0.70,0.85); //light blue like old 5D sensors
    toolNode->Modified();

    m_Visualizer->SetRepresentationObject( toolID, toolNode->GetData()); // updating node with changed surface back in visualizer

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  catch(std::exception& e) 
  {
    QMessageBox::warning(NULL, QString("IGTSurfaceTracker: Error"), QString("Can not change tool representation!").arg(e.what()));
    return;
  }
}


QWidget* QmitkIGTTrackingLabView::CreatePointSetRecordingWidget(QWidget* parent)
{
  QWidget* pointSetRecordingWidget = new QWidget(parent);
  m_PSRecToolSelectionComboBox = new QComboBox(pointSetRecordingWidget);
  m_PSRecordingSpinBox = new QSpinBox(pointSetRecordingWidget);
  QLabel* psRecordingEpsilonDistance = new QLabel("mm (point distance)", pointSetRecordingWidget);
   
  // the recording button
  m_PointSetRecordPushButton = new QPushButton("Start PointSet Recording", pointSetRecordingWidget);
  m_PointSetRecordPushButton->setDisabled(true);
  m_PointSetRecordPushButton->setIcon(QIcon(":/QmitkQmitkIGTTrackingLabView/start_rec.png"));
  m_PointSetRecordPushButton->setCheckable(true);
  connect( m_PointSetRecordPushButton, SIGNAL(toggled(bool)), this, SLOT(OnPointSetRecording(bool)) );
  
  // distances spin
  m_PSRecordingSpinBox->setValue(1);
  m_PSRecordingSpinBox->setMinimum(1);
  m_PSRecordingSpinBox->setMaximum(20);
  
  QLabel* toolSelectLabel = new QLabel("Select tool for recording:", pointSetRecordingWidget);
  QGridLayout* layout = new QGridLayout(pointSetRecordingWidget);
  
  int row = 0;
  int col = 0;

  layout->addWidget(toolSelectLabel,row,col++,1,1,Qt::AlignRight);
  layout->addWidget(m_PSRecToolSelectionComboBox,row,col++,1,3,Qt::AlignLeft);
  col +=2;
  layout->addWidget(m_PSRecordingSpinBox,row,col++,1,1,Qt::AlignRight);
  layout->addWidget(psRecordingEpsilonDistance, row, col++,1,1,Qt::AlignLeft);
  
  row++;
  col=4;

  layout->addWidget(m_PointSetRecordPushButton,row,col++,1,2,Qt::AlignRight);

  return pointSetRecordingWidget;
}

void QmitkIGTTrackingLabView::OnPointSetRecording(bool record)
{
  mitk::DataStorage* ds = this->GetDefaultDataStorage();
  
  if(ds == NULL)
    return;

  if(record)
  {
    mitk::DataNode::Pointer psRecND = ds->GetNamedNode(m_PointSetRecordingDataNodeName);
    if(m_PSRecordingPointSet.IsNull() || psRecND.IsNull())
    {
      m_PSRecordingPointSet = NULL;
      m_PSRecordingPointSet = mitk::PointSet::New();
      mitk::DataNode::Pointer dn = mitk::DataNode::New();
      dn->SetName(m_PointSetRecordingDataNodeName);
      dn->SetColor(0.,1.,0.);
      dn->SetData(m_PSRecordingPointSet);
      ds->Add(dn);
    }
    else
      m_PSRecordingPointSet->Clear();

    m_PointSetRecording = true;
    m_PointSetRecordPushButton->setText("Stop PointSet Recording");
    m_PSRecToolSelectionComboBox->setDisabled(true);
  }

  else
  {
    m_PointSetRecording = false;
    m_PointSetRecordPushButton->setText("Start PointSet Recording");
    m_PSRecToolSelectionComboBox->setEnabled(true);
  }
}

void QmitkIGTTrackingLabView::DestroyInstrumentVisualization(mitk::DataStorage* ds, mitk::TrackingDevice::Pointer tracker)
{
  if(ds == NULL || tracker.IsNull())
    return;

  for(int i=0; i < tracker->GetToolCount(); ++i)
  {
    mitk::DataNode::Pointer dn = ds->GetNamedNode(tracker->GetTool(i)->GetToolName());

    if(dn.IsNotNull())
      ds->Remove(dn);
  }
}


void QmitkIGTTrackingLabView::GlobalReinit()
{
  // request global reiinit
  mitk::NodePredicateNot::Pointer pred = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("includeInBoundingBox", mitk::BoolProperty::New(false)));
  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetDataStorage()->GetSubset(pred);

  // calculate bounding geometry of these nodes
  mitk::TimeSlicedGeometry::Pointer bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(rs, "visible");

  // initialize the views to the bounding geometry
  mitk::RenderingManager::GetInstance()->InitializeViews(bounds);

  //global reinit end
}

void QmitkIGTTrackingLabView::OnVirtualCamera(int toolNr, bool on)
{
   if(m_VirtualView.IsNull() || m_FiducialRegistrationFilter.IsNull())
    return;

   if(on)
   {
     m_VirtualView->SetInput(m_FiducialRegistrationFilter->GetOutput(toolNr));
     this->GetActiveStdMultiWidget()->SetWidgetPlaneModeToRotation(true);
   }
   else
     this->GetActiveStdMultiWidget()->SetWidgetPlaneModeToSlicing(true);

}


void QmitkIGTTrackingLabView::OnPermanentRegistration(int toolID, bool on)
{
  if (m_PermanentRegistrationFilter.IsNull() || m_FiducialRegistrationFilter.IsNull())
    return;

  if(on)
  {
    if(m_PermanentRegistrationSourcePoints.IsNull())
      m_PermanentRegistrationSourcePoints = mitk::PointSet::New();

    // interconnectiong permanent registration filter between tracking source and fiducial registration filter
    for(unsigned int i=0; i < m_Source->GetNumberOfOutputs(); ++i)
    {
        m_PermanentRegistrationFilter->SetInput(i,m_Source->GetOutput(i));
        m_FiducialRegistrationFilter->SetInput(i,m_PermanentRegistrationFilter->GetOutput(i));
    }

    mitk::NavigationData::Pointer nd = m_Source->GetOutput((unsigned int) toolID);
    
    m_PermanentRegistrationFilter->SetTargetLandmarks(this->GetVirtualPointSetFromPosition(nd));
  }

  else
  {
    for(unsigned int i=0; i < m_FiducialRegistrationFilter->GetNumberOfOutputs(); ++i)
      m_FiducialRegistrationFilter->SetInput(i,m_Source->GetOutput());
  }
    
}

mitk::PointSet::Pointer QmitkIGTTrackingLabView::GetVirtualPointSetFromPosition(mitk::NavigationData::Pointer navigationData)
{
  typedef itk::QuaternionRigidTransform<double> QuaternionTransformType;

  mitk::NavigationData::PositionType pointA;
  mitk::NavigationData::PositionType pointB;
  mitk::NavigationData::PositionType pointC;

  //initializing three points with position(0|0|0)
  pointA.Fill(0);
  pointB.Fill(0);
  pointC.Fill(0);

  // changing position off all points in order to make them orthogonal
  pointA[0] = 1;
  pointB[1] = 1;
  pointC[2] = 1;

  QuaternionTransformType::Pointer quatTransform = QuaternionTransformType::New();

  // orientation of NavigationData from parameter
  mitk::NavigationData::OrientationType quatIn = navigationData->GetOrientation();
  
  // set orientation to quaternion transform
  vnl_quaternion<double> const vnlQuatIn(quatIn.x(), quatIn.y(), quatIn.z(), quatIn.r());
  quatTransform->SetRotation(vnlQuatIn);

  // transform each point
  pointA = quatTransform->TransformPoint(pointA);
  pointB = quatTransform->TransformPoint(pointB);
  pointC = quatTransform->TransformPoint(pointC);

  // add position data from NavigationData parameter to each point
  pointA[0] += navigationData->GetPosition()[0];
  pointA[1] += navigationData->GetPosition()[1];
  pointA[2] += navigationData->GetPosition()[2];
  
  pointB[0] += navigationData->GetPosition()[0];
  pointB[1] += navigationData->GetPosition()[1];
  pointB[2] += navigationData->GetPosition()[2];
  
  pointC[0] += navigationData->GetPosition()[0];
  pointC[1] += navigationData->GetPosition()[1];
  pointC[2] += navigationData->GetPosition()[2];

  // insert points in source points pointset for the permanent registration landmark transform
  m_PermanentRegistrationSourcePoints->InsertPoint(0,pointA);
  m_PermanentRegistrationSourcePoints->InsertPoint(1,pointB);
  m_PermanentRegistrationSourcePoints->InsertPoint(2,pointC);


  return m_PermanentRegistrationSourcePoints;
}

