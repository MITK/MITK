/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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
#include "QmitkUpdateTimerWidget.h"
#include "QmitkToolDistanceWidget.h"
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
#include <mitkStatusBar.h>


#include <QMessageBox>
#include <qtimer.h>


#define WARN LOG_WARN("ToolPairNavigation")

const std::string QmitkToolPairNavigationView::VIEW_ID = "org.mitk.views.toolpairnavigation";

QmitkToolPairNavigationView::QmitkToolPairNavigationView()
: QmitkFunctionality(), m_MultiWidget(NULL)
{
}

QmitkToolPairNavigationView::~QmitkToolPairNavigationView()
{

  delete m_NDIConfigWidget;
  delete m_DistanceWidget;
  m_NDIConfigWidget = NULL;
  m_DistanceWidget = NULL;
  m_Source = NULL;
  m_Visualizer = NULL;
  m_CameraVisualizer = NULL;
 
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
  m_DistanceWidget = new QmitkToolDistanceWidget(parent);


  // removes all placeholder tabs from the toolbox that where created in the qt designer before
  int tabnr = this->m_Controls.m_ToolBox->count();
  for(int i=0; i < tabnr ;i++)
  {
    this->m_Controls.m_ToolBox->removeItem(0);
  }

  // inserts this bundle's widgets into the toolbox
  this->m_Controls.m_ToolBox->insertItem(0,m_NDIConfigWidget,QString("Configuration"));
  this->m_Controls.m_ToolBox->insertItem(1,m_DistanceWidget,QString("Distances"));



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
  connect( m_NDIConfigWidget, SIGNAL(Connected()), this, SLOT(SetNavigationUp()));
  connect( m_NDIConfigWidget, SIGNAL(Connected()), this->m_Controls.m_RenderingTimerWidget, SLOT(EnableWidget()));
  connect( m_NDIConfigWidget, SIGNAL(Disconnected()), this, SLOT(Disconnected()));
  connect( m_NDIConfigWidget, SIGNAL(Disconnected()), this->m_Controls.m_RenderingTimerWidget, SLOT(DisableWidget()));


  //to be implemented for tool name changig e.g.
  //  connect(m_NDIConfigWidget, SIGNAL(ToolsChanged()), this, SLOT(ToolsChanged()));
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
  if(m_Controls.m_RenderingTimerWidget != NULL)
  {
    this->m_Controls.m_RenderingTimerWidget->StopTimer();
  }
  if(m_Source.IsNotNull() && m_Source->IsTracking())
  {
    this->m_Source->StopTracking();
  }
  this->DestroyIGTPipeline();
  this->RemoveVisualizationObjects(this->GetDefaultDataStorage());

  if(this->m_DistanceWidget != NULL)
    this->m_DistanceWidget->ClearDistanceMatrix();
}

void QmitkToolPairNavigationView::ToolsAdded( QStringList tools )
{

}


void QmitkToolPairNavigationView::SetNavigationUp()
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

  //2. start IGT pipeline to display tracking devices (20 Hz update rate -> 50 msec timeout)
  try
  {
    // setup for the bundle's update timer widget
    m_Controls.m_RenderingTimerWidget->SetPurposeLabelText(QString("Navigation"));
    m_Controls.m_RenderingTimerWidget->SetTimerInterval( 50 );

    connect( m_Controls.m_RenderingTimerWidget->GetUpdateTimer() , SIGNAL(timeout()) , this, SLOT (RenderScene()) );
    connect( m_Controls.m_RenderingTimerWidget, SIGNAL(Started()), this, SLOT(StartNavigation()));
    connect( m_Controls.m_RenderingTimerWidget, SIGNAL(Stopped()) , this, SLOT (StopNavigation()));
    connect( m_Controls.m_RenderingTimerWidget, SIGNAL(Stopped()) , m_DistanceWidget, SLOT (SetDistanceLabelValuesInvalid()));

  }
  catch(std::exception& e)
  {
    QMessageBox::warning(NULL, "ToolPairNavigation: Error", QString("Error while starting the IGT-Pipeline: %1").arg(e.what()));
    this->m_Controls.m_RenderingTimerWidget->StopTimer();
    this->DestroyIGTPipeline();
    return;
  }
}

void QmitkToolPairNavigationView::StartNavigation()
{
  if(m_Source.IsNotNull() && !m_Source->IsTracking())
  {
    m_Source->StartTracking();
    // creates the matrix with distances from the tracking source's outputs
    m_DistanceWidget->CreateToolDistanceMatrix(m_Source->GetOutputs());
  }
}


// is for tool changing events, like name changes
void QmitkToolPairNavigationView::ToolsChanged()
{

}

void QmitkToolPairNavigationView::StopNavigation()
{
  if(m_Source.IsNotNull() && m_Source->IsTracking())
  {
    m_Source->StopTracking();
  }
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
    mitk::DataNode::Pointer toolrepresentationNode = ds->GetNamedNode(toolName);
    if (toolrepresentationNode.IsNull())
    {
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
    }

    m_Visualizer->SetRepresentationObject(i, toolrepresentationNode->GetData());  // set instrument nodes as baseData for visualisation filter  
  }


}

void QmitkToolPairNavigationView::DestroyIGTPipeline()
{
  if (m_Source.IsNotNull())
  {
    m_Source->StopTracking();
    m_Source->Disconnect();
    m_Source = NULL;
  }  

  m_Visualizer = NULL;
  m_CameraVisualizer = NULL;
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


void QmitkToolPairNavigationView::RenderScene()
{
  try
  {
    if (m_Visualizer.IsNull() || m_CameraVisualizer.IsNull() || this->GetActiveStdMultiWidget() == NULL)
      return;
    try
    {
      m_Visualizer->Update();
      m_CameraVisualizer->Update();

      //every tenth update
      static int counter = 0;
      if (counter > 9)
      {
        this->m_DistanceWidget->ShowDistanceValues(m_Source->GetOutputs());
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

