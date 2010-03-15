/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Do, 28 Mai 2009) $
Version:   $Revision: 17495 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkToolPairNavigationView.h"
#include "QmitkNDIConfigurationWidget.h"
#include "mitkNodePredicateDataType.h"
#include <mitkTrackingVolume.h>
#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"
#include <mitkCone.h>
#include <vtkConeSource.h>
#include <mitkEllipsoid.h>
#include <vtkSphereSource.h>
#include <mitkProperties.h>
#include <mitkNodePredicateProperty.h>

#include <QMessageBox>
#include <qtimer.h>


#define WARN LOG_WARN("ToolPairNavigation")

const std::string QmitkToolPairNavigationView::VIEW_ID = "org.mitk.views.toolpairnavigation";

QmitkToolPairNavigationView::QmitkToolPairNavigationView()
: QmitkFunctionality(), m_MultiWidget(NULL), m_RenderingTimer(NULL)
{
}

QmitkToolPairNavigationView::~QmitkToolPairNavigationView()
{
}

void QmitkToolPairNavigationView::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets
  m_Controls.setupUi(parent);
  this->CreateBundleWidgets(parent);
  this->CreateConnections();
}

void QmitkToolPairNavigationView::CreateBundleWidgets(QWidget* parent)
{
  //get default data storage
  //mitk::DataStorage* ds = this->GetDefaultDataStorage();

  //instanciate widget
  m_NDIConfigWidget = new QmitkNDIConfigurationWidget(parent);

  m_Controls.m_ToolBox->insertItem(0, m_NDIConfigWidget, "Configuration");
}

void QmitkToolPairNavigationView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}

void QmitkToolPairNavigationView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkToolPairNavigationView::CreateConnections()
{
  connect( (QObject*)(m_Controls.m_pbStartNavigation), SIGNAL(clicked()), this, SLOT(OnStartNavigation()) );
  connect( (QObject*)(m_Controls.m_pbPauseNavigation), SIGNAL(clicked(bool)), this, SLOT(OnPauseNavigation(bool)) );
  connect( (QObject*)(m_Controls.m_pbStopNavigation), SIGNAL(clicked()), this, SLOT(OnStopNavigation()) );

  //NDI configuration widget
  //connect( (QObject*)(m_NDIConfigWidget), SIGNAL(Disconnected()), this, SLOT(Disconnected()) );
  //connect( (QObject*)(m_NDIConfigWidget), SIGNAL(ToolsAdded(QStringList)), this, SLOT(ToolsAdded(QStringList)) );
}

void QmitkToolPairNavigationView::Activated()
{
  QmitkFunctionality::Activated();

  //switch to 3D rendering mode in Widget3
  m_LastMapperIDWidget3 = mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget3->GetRenderWindow())->GetMapperID();
  mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget3->GetRenderWindow())->SetMapperID(2);
  mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget2->GetRenderWindow())->SetMapperID(2);
}

void QmitkToolPairNavigationView::Deactivated()
{
  QmitkFunctionality::Deactivated();

  //switch back cameravisualization view in Widget3
  mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget3->GetRenderWindow())->SetMapperID(m_LastMapperIDWidget3);
  mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget2->GetRenderWindow())->SetMapperID(1);
}

void QmitkToolPairNavigationView::Disconnected()
{

}

void QmitkToolPairNavigationView::ToolsAdded( QStringList tools )
{

}


void QmitkToolPairNavigationView::OnStartNavigation()
{
  if (m_Source.IsNotNull())
    if (m_Source->IsTracking())
      return;

  mitk::DataStorage* ds = this->GetDefaultDataStorage();
  if (ds == NULL)
  {
    QMessageBox::warning(NULL, "ToolPairNavigation: Error", "can not access DataStorage. Navigation not possible");
    return;
  }

  //1. build up the filter pipeline
  try
  {  
    this->SetupIGTPipeline(); 
  }
  catch(std::exception& e)
  {
    QMessageBox::warning(NULL, "ToolPairNavigation: Error", QString("Error while building the IGT-Pipeline: %1").arg(e.what()));
    this->DestroyIGTPipeline();
    return;
  }
  catch (...)
  {
    QMessageBox::warning(NULL, "ToolPairNavigation: Error", QString("Error while building the IGT-Pipeline."));
    this->DestroyIGTPipeline();
    return;
  }

  //2. start IGT pipeline to display tracking devices (20 Hz update rate)
  try
  {
    this->StartContinuousUpdate(20);  
    m_Controls.m_NavigationStatusDisplay->setText("Navigation started. Searching instruments...");
  }
  catch(std::exception& e)
  {
    QMessageBox::warning(NULL, "ToolPairNavigation: Error", QString("Error while starting the IGT-Pipeline: %1").arg(e.what()));
    this->StopContinuousUpdate();
    this->DestroyIGTPipeline();
    m_Controls.m_NavigationStatusDisplay->setText(m_NavigationOfflineText);
    return;
  }
}

void QmitkToolPairNavigationView::OnPauseNavigation( bool pause )
{
  if ( m_RenderingTimer == NULL)
    return;
  if (pause == true)
  {
    m_RenderingTimer->stop();
    m_Controls.m_NavigationStatusDisplay->setText("Navigation is paused. There are currently no position updates!");
  }
  else
    m_RenderingTimer->start();
}

void QmitkToolPairNavigationView::OnStopNavigation()
{
  this->StopContinuousUpdate();
  this->DestroyIGTPipeline();

  this->RemoveVisualizationObjects(this->GetDefaultDataStorage());

  /* reset GUI */
  m_Controls.m_NavigationStatusDisplay->setText(m_NavigationOfflineText);
}

void QmitkToolPairNavigationView::OnShowTrackingVolume( bool on )
{
  if (m_Source->GetTrackingDevice() == NULL)
  {
    QMessageBox::warning(NULL, "Warning", QString("Please connect tracker first \n"));
    return;
  }

  if (m_Source->GetTrackingDevice()->GetState() == mitk::TrackingDevice::Setup)
  {
    QMessageBox::warning(NULL, "Warning", QString("Please start connect tracker first\n"));
    return;
  }

  mitk::DataStorage* ds = this->GetDefaultDataStorage();
  if (ds == NULL)
    return;

  //try to the the node
  std::string trackerVolumeName = "Tracking Volume";
  mitk::DataNode::Pointer node = ds->GetNamedNode(trackerVolumeName);

  if (on == false)
  {
    //if the node is present
    if (node.IsNotNull())
      node->SetVisibility(false);
  }
  else
  {
    //no node present yet
    if (node.IsNull())
    {
      // tracking volume
      mitk::TrackingVolume::Pointer volume = mitk::TrackingVolume::New();
      if (volume->SetTrackingDeviceType(m_Source->GetTrackingDevice()->GetType()) == false)
      {
        QMessageBox::warning(NULL, "Error", "can not load tracking volume. Tracking volume display is not available");
        return;
      }
      mitk::DataNode::Pointer node = mitk::DataNode::New();
      node->SetData(volume);
      node->SetName(trackerVolumeName);
      node->SetOpacity(0.2);
      node->SetColor(0.4, 0.4, 1.0);
      node->SetBoolProperty("helperObject", true);
      node->SetBoolProperty("ToolPairNavigation", true);
      ds->Add(node);
    }
    else
    {
      //mark as visible
      node->SetVisibility(true);
    }

    // hide widget planes 
    if (this->GetActiveStdMultiWidget() != NULL)
      this->GetActiveStdMultiWidget()->SetWidgetPlanesVisibility(false);  
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

}

void QmitkToolPairNavigationView::SetupIGTPipeline()
{
  mitk::DataStorage* ds = this->GetDefaultDataStorage(); // check if dataStorage is available
  if (ds == NULL)
    throw std::invalid_argument("DataStorage not available");

  mitk::TrackingDevice::Pointer tracker = m_NDIConfigWidget->GetTracker(); // get current tracker from the configuration widget
  if (tracker.IsNull()) // check if tracker is valid
    throw std::invalid_argument("tracking device is NULL.");

  m_Source = mitk::TrackingDeviceSource::New(); // create a new source for the IGT filter pipeline
  m_Source->SetTrackingDevice(tracker); // set the found tracker to the source

  m_MessageFilter = mitk::NavigationDataToMessageFilter::New(); // filter used to display messages related to NDs
  m_Visualizer = mitk::NavigationDataObjectVisualizationFilter::New(); // filter to display NDs
  m_CameraVisualizer = mitk::CameraVisualization::New();
  //set widget 3
  m_CameraVisualizer->SetRenderer(mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget3->GetRenderWindow()));
  
  //set viewing direction
  mitk::Vector3D viewVector;
  mitk::FillVector3D( viewVector, 0.0, 0.0, 1.0 );
  m_CameraVisualizer->SetDirectionOfProjectionInToolCoordinates(viewVector);

  /* prepare visualization objects and filter */
  for (unsigned int i = 0; i < m_Source->GetNumberOfOutputs(); ++i) //for each connected tool 
  {    
    m_Visualizer->SetInput(i, m_Source->GetOutput(i)); // set input for visualization filter

    const char* toolName = tracker->GetTool(i)->GetToolName();
    mitk::DataNode::Pointer toolrepresentationNode;
    
    //the first tool represents the tool to guide 
    //it will be represented as cone wheras the target tools will be represented by a sphere
    if (i<1) //tool to guide
    {
      toolrepresentationNode = this->CreateConeAsInstrumentVisualization(toolName);
      m_CameraVisualizer->SetInput(m_Source->GetOutput(i));      
    }
    else
      toolrepresentationNode = this->CreateSphereAsInstrumentVisualization(toolName);

    ds->Add(toolrepresentationNode);
    m_Visualizer->SetRepresentationObject(i, toolrepresentationNode->GetData());  // set instrument nodes as baseData for visualisation filter  
    m_MessageFilter->SetInput(i, m_Source->GetOutput(i));  // set input for message filter
  }
  

  m_MessageFilter->AddDataValidChangedListener(mitk::MessageDelegate2<QmitkToolPairNavigationView, bool, unsigned int>(this, &QmitkToolPairNavigationView::OnDataValidChanged));
}

void QmitkToolPairNavigationView::DestroyIGTPipeline()
{
  if (m_Source.IsNotNull())
  {
    m_Source->StopTracking();
    m_Source->Disconnect();
    m_Source = NULL;
  }  

  if (m_MessageFilter.IsNotNull())
  {
    m_MessageFilter->RemoveDataValidChangedListener(mitk::MessageDelegate2<QmitkToolPairNavigationView, bool, unsigned int>(this, &QmitkToolPairNavigationView::OnDataValidChanged));
    m_MessageFilter = NULL;
  }

  m_Visualizer = NULL;
  m_CameraVisualizer = NULL;
}

void QmitkToolPairNavigationView::StartContinuousUpdate(unsigned int frameRate)
{
  if (m_Source.IsNull() || m_Visualizer.IsNull() || m_CameraVisualizer.IsNull())
    throw std::invalid_argument("Pipeline is not set up correctly");
  if (frameRate == 0)
    throw std::invalid_argument("framerate must be larger than 0");

  /* start continuous update */
  m_Source->StartTracking();

  if (m_RenderingTimer == NULL)
    m_RenderingTimer = new QTimer(this);
  connect(m_RenderingTimer, SIGNAL(timeout()), this, SLOT(RenderScene())); //connect the timer to the method RenderScene()
  m_RenderingTimer->setInterval(1000/frameRate);
  m_RenderingTimer->start();

}

void QmitkToolPairNavigationView::StopContinuousUpdate()
{
  /* stop continuous update */
  if (m_RenderingTimer != NULL)
  {
    m_RenderingTimer->stop();
    disconnect(m_RenderingTimer, SIGNAL(timeout()), this, SLOT(RenderScene()));
    delete m_RenderingTimer;
    m_RenderingTimer = NULL;
  }
}


mitk::DataNode::Pointer QmitkToolPairNavigationView::CreateConeAsInstrumentVisualization(const char* label)
{
  //new data
  mitk::Cone::Pointer activeToolData = mitk::Cone::New();
  vtkConeSource* vtkData = vtkConeSource::New();
  vtkData->SetRadius(3.0);
  vtkData->SetHeight(6.0);
  vtkData->SetDirection(0.0, 0.0, 1.0); //(0.0, 0.0, -1.0) for 5DoF
  vtkData->SetCenter(0.0, 0.0, 0.0);
  vtkData->SetResolution(20);
  vtkData->CappingOn();
  vtkData->Update();
  activeToolData->SetVtkPolyData(vtkData->GetOutput());
  vtkData->Delete();

  //new node
  mitk::DataNode::Pointer coneNode = mitk::DataNode::New();
  coneNode->SetData(activeToolData);
  coneNode->GetPropertyList()->SetProperty("name", mitk::StringProperty::New ( label ) );
  coneNode->GetPropertyList()->SetProperty("layer", mitk::IntProperty::New(0));
  coneNode->GetPropertyList()->SetProperty("visible",mitk::BoolProperty::New(true));
  //don't display in widget 3 (3D camera view)
  coneNode->SetVisibility(false, mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget3->GetRenderWindow())); 
  coneNode->SetColor(1.0,0.0,0.0);//red
  coneNode->SetOpacity(0.7);
  coneNode->Modified();

  return coneNode;
}

mitk::DataNode::Pointer QmitkToolPairNavigationView::CreateSphereAsInstrumentVisualization(const char* label)
{
  //new data
  mitk::Ellipsoid::Pointer activeToolData = mitk::Ellipsoid::New();
  vtkSphereSource *vtkData = vtkSphereSource::New();
  vtkData->SetRadius(1.0f);
  vtkData->SetCenter(0.0, 0.0, 0.0);
  vtkData->Update();
  activeToolData->SetVtkPolyData(vtkData->GetOutput());
  vtkData->Delete();

  //new node
  mitk::DataNode::Pointer sphereNode = mitk::DataNode::New();
  sphereNode->SetData(activeToolData);
  sphereNode->GetPropertyList()->SetProperty("name", mitk::StringProperty::New ( label ) );
  sphereNode->GetPropertyList()->SetProperty("layer", mitk::IntProperty::New(0));
  sphereNode->GetPropertyList()->SetProperty("visible",mitk::BoolProperty::New(true));
  sphereNode->SetColor(0.0,1.0,0.0);//green
  sphereNode->SetOpacity(1.0);
  sphereNode->Modified();

  return sphereNode;
}

void QmitkToolPairNavigationView::OnDataValidChanged(bool newValue, unsigned int index)
{
  QString header("<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0//EN' 'http://www.w3.org/TR/REC-html40/strict.dtd'>"
    "<html><head><meta name='qrichtext' content='1' />"
    "</head><body style=' font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;'>");
  QString footer("</body></html>");

  QString okay("%1 is tracked <span style=' font-weight:600; color:#2f9f00;'>OKAY</span>"); 
  QString cannotbetracked( "%1 <span style='font-weight:600; color:#cc0104;'>can not be tracked!</span>");

  if (m_Source.IsNull())
    return; // no need to display anything, if tracking isn't even setup.
  if (m_Source->IsTracking() == false)
    return; // no need to display anything, if tracking isn't even setup.

  mitk::TrackingDevice::ConstPointer tracker = m_Source->GetTrackingDevice();
  QString dataValidOutput = header;
  for (unsigned int i = 0; i < m_MessageFilter->GetNumberOfOutputs(); ++i)  // retrieve validity of each input and build up status string accordingly
  {
    const char* toolName = tracker->GetTool(i)->GetToolName();
    if (m_MessageFilter->GetInput(i)->IsDataValid())
      dataValidOutput += okay.arg(QString(toolName));
    else
      dataValidOutput += cannotbetracked.arg(QString(toolName));
    dataValidOutput += QString("<BR/>");
  }
  dataValidOutput += footer;
  m_Controls.m_NavigationStatusDisplay->setText(dataValidOutput);
}

void QmitkToolPairNavigationView::RenderScene()
{
  try
  {
    if (m_Visualizer.IsNull() || m_MessageFilter.IsNull() || m_CameraVisualizer.IsNull() || this->GetActiveStdMultiWidget() == NULL)
      return;
    try
    {
      m_Visualizer->Update();
      m_MessageFilter->Update(); // can be moved into own thread with lower update rate
      m_CameraVisualizer->Update();
      
      //every tenth update
      static int counter = 0;
      if (counter > 9)
      {
        //compute distance from tool 1 and tool 2 and display it 
        mitk::NavigationData* nd0 = m_Source->GetOutput(0);
        mitk::NavigationData* nd1 = m_Source->GetOutput(1);
        mitk::NavigationData::PositionType::RealType distanceToTool =  nd0->GetPosition().EuclideanDistanceTo(nd1->GetPosition());
        QString str;
        str.setNum(distanceToTool);
        str+= " mm";
        m_Controls.m_TLDistance->setText(str);
        counter = 0;
      }
      else
        counter++;
    }
    catch(std::exception& e) 
    {
      std::cout << "Exception during QmitkToolPairNavigationView::RenderScene():" << e.what() << "\n";
    }
    
    //update all Widgets
    mitk::RenderingManager::GetInstance()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
  }
  catch (std::exception& e)
  {
    std::cout << "RenderAll exception: " << e.what() << "\n";
  }
  catch (...)
  {
    std::cout << "RenderAll unknown exception\n";
  }
}

void QmitkToolPairNavigationView::RemoveVisualizationObjects( mitk::DataStorage* ds )
{
  if (ds != NULL)
    ds->Remove(ds->GetSubset(mitk::NodePredicateProperty::New("ToolPairNavigation", mitk::BoolProperty::New(true)))); // remove all objects that have the ToolPairNavigation tag

}

