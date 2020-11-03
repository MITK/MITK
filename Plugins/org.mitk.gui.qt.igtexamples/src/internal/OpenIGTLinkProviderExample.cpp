/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkRenderWindow.h"

// Qt
#include <QMessageBox>
#include <QFileDialog>

// mitk
#include <mitkImage.h>
#include <mitkSurface.h>
//#include <mitkIGTLMessageToNavigationDataFilter.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkIOUtil.h>

// vtk
#include <vtkSphereSource.h>

//
#include "OpenIGTLinkProviderExample.h"

//igtl
#include "igtlStringMessage.h"


const std::string OpenIGTLinkProviderExample::VIEW_ID =
    "org.mitk.views.OpenIGTLinkProviderExample";

OpenIGTLinkProviderExample::~OpenIGTLinkProviderExample()
{
   this->DestroyPipeline();
   if (m_IGTLMessageProvider.IsNotNull())
   {
      m_IGTLMessageProvider->UnRegisterMicroservice();
   }
   if (m_NavDataToIGTLMsgFilter.IsNotNull())
   {
      m_NavDataToIGTLMsgFilter->UnRegisterMicroservice();
   }
}

void OpenIGTLinkProviderExample::SetFocus()
{
}

void OpenIGTLinkProviderExample::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  // connect the widget items with the methods
  connect( m_Controls.butStart, SIGNAL(clicked()),
           this, SLOT(Start()) );
  connect( m_Controls.butOpenNavData, SIGNAL(clicked()),
           this, SLOT(OnOpenFile()) );
  connect( &m_VisualizerTimer, SIGNAL(timeout()),
           this, SLOT(UpdateVisualization()));


  //create a new OpenIGTLink Client
  m_IGTLServer = mitk::IGTLServer::New(false);
  m_IGTLServer->SetName("OIGTL Provider Example Device");

  //create a new OpenIGTLink Device source
  m_IGTLMessageProvider = mitk::IGTLMessageProvider::New();

  //set the OpenIGTLink server as the source for the device source
  m_IGTLMessageProvider->SetIGTLDevice(m_IGTLServer);

  //register the provider so that it can be configured with the IGTL manager
  //plugin. This could be hardcoded but now I already have the fancy plugin.
  m_IGTLMessageProvider->RegisterAsMicroservice();
}

void OpenIGTLinkProviderExample::CreatePipeline()
{
  //create a navigation data player object that will play nav data from a
  //recorded file
  m_NavDataPlayer = mitk::NavigationDataSequentialPlayer::New();

  //set the currently read navigation data set
  m_NavDataPlayer->SetNavigationDataSet(m_NavDataSet);

  //create a filter that converts navigation data into IGTL messages
  m_NavDataToIGTLMsgFilter = mitk::NavigationDataToIGTLMessageFilter::New();

  //connect the filters with each other
  //the navigation data player reads a file with recorded navigation data,
  //passes this data to a filter that converts it into a IGTLMessage.
  //The provider is not connected because it will search for fitting services.
  //Once it found the filter it will automatically connect to it.
  m_NavDataToIGTLMsgFilter->ConnectTo(m_NavDataPlayer);

  //define the operation mode for this filter, we want to send tracking data
  //messages
  m_NavDataToIGTLMsgFilter->SetOperationMode(
    mitk::NavigationDataToIGTLMessageFilter::ModeSendTDataMsg);
  //       mitk::NavigationDataToIGTLMessageFilter::ModeSendTransMsg);

  //set the name of this filter to identify it easier
  m_NavDataToIGTLMsgFilter->SetName("Tracking Data Source From Example");

  //register this filter as micro service. The message provider looks for
  //provided IGTLMessageSources, once it found this microservice and someone
  //requested this data type then the provider will connect with this filter
  //automatically.
  m_NavDataToIGTLMsgFilter->RegisterAsMicroservice();

  //also create a visualize filter to visualize the data
  m_NavDataVisualizer = mitk::NavigationDataObjectVisualizationFilter::New();

  //create an object that will be moved respectively to the navigation data
  for (size_t i = 0; i < m_NavDataPlayer->GetNumberOfIndexedOutputs(); i++)
  {
    mitk::DataNode::Pointer newNode = mitk::DataNode::New();
    QString name("DemoNode IGTLProviderExmpl T");
    name.append(QString::number(i));
    newNode->SetName(name.toStdString());

    //create small sphere and use it as surface
    mitk::Surface::Pointer mySphere = mitk::Surface::New();
    vtkSmartPointer<vtkSphereSource> vtkData = vtkSmartPointer<vtkSphereSource>::New();
    vtkData->SetRadius(2.0f);
    vtkData->SetCenter(0.0, 0.0, 0.0);
    vtkData->Update();
    mySphere->SetVtkPolyData(vtkData->GetOutput());
    newNode->SetData(mySphere);

    this->GetDataStorage()->Add(newNode);

    m_NavDataVisualizer->SetRepresentationObject(i, mySphere.GetPointer());

    m_DemoNodes.append(newNode);
  }

  //connect the visualization with the navigation data player
  m_NavDataVisualizer->ConnectTo(m_NavDataPlayer);

  //start the player
  this->Start();

  //resize the boundaries
  this->m_NavDataVisualizer->Update();
  this->ResizeBoundingBox();
}

void OpenIGTLinkProviderExample::DestroyPipeline()
{
   if (m_NavDataPlayer.IsNotNull())
   {
      //m_NavDataPlayer->StopPlaying();
   }
   foreach(mitk::DataNode::Pointer node, m_DemoNodes)
   {
      this->GetDataStorage()->Remove(node);
   }
   this->m_DemoNodes.clear();
}

void OpenIGTLinkProviderExample::Start()
{
  if ( this->m_Controls.butStart->text().contains("Start") )
  {
    m_NavDataPlayer->SetRepeat(true);
    //m_NavDataPlayer->StartPlaying();
    this->m_Controls.butStart->setText("Stop Playing Recorded Navigation Data ");

    //start the visualization
    double fps = m_Controls.m_updateRate->value();
    double millisecondsPerFrame = 1 / fps * 1000;

    this->m_VisualizerTimer.start(millisecondsPerFrame);
  }
  else
  {
    //m_NavDataPlayer->StopPlaying();
    this->m_Controls.butStart->setText("Start Playing Recorded Navigation Data ");

    //stop the visualization
    this->m_VisualizerTimer.stop();
  }
}

void OpenIGTLinkProviderExample::OnOpenFile(){

  // FIXME Filter for correct files and use correct Reader
  QString filter = tr("NavigationData File (*.csv *.xml)");
  QString fileName = QFileDialog::getOpenFileName(nullptr, tr("Open NavigationData Set"), "", filter);

  if ( fileName.isNull() ) { return; } // user pressed cancel

  try
  {
    m_NavDataSet = dynamic_cast<mitk::NavigationDataSet*> (mitk::IOUtil::Load(fileName.toStdString())[0].GetPointer());
  }
  catch ( const mitk::Exception &e )
  {
    MITK_WARN("NavigationDataPlayerView") << "could not open file " << fileName.toStdString();
    QMessageBox::critical(nullptr, "Error Reading File", "The file '" + fileName
                          +"' could not be read.\n" + e.GetDescription() );
    return;
  }

  this->m_Controls.butStart->setEnabled(true);

  //Setup the pipeline
  this->CreatePipeline();

  // Update Labels
//  m_Controls->m_LblFilePath->setText(fileName);
//  m_Controls->m_LblTools->setText(QString::number(m_NavDataSet->GetNumberOfTools()));
}

void OpenIGTLinkProviderExample::UpdateVisualization()
{
  this->m_NavDataPlayer->GoToNextSnapshot();
  if (this->m_Controls.visualizeCheckBox->isChecked())
  {
    //update the filter
    this->m_NavDataVisualizer->Update();

    ////update the bounds
    //mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());

    //update rendering
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

/**
* \brief To initialize the scene to the bounding box of all visible objects
*/
void OpenIGTLinkProviderExample::ResizeBoundingBox()
{
  // get all nodes
  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetDataStorage()->GetAll();
  auto bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(rs)->Clone();

  if (bounds.IsNull())
  {
    return;
  }

  //expand the bounding box in case the instruments are all at one position
  mitk::Point3D center = bounds->GetCenterInWorld();
  mitk::Geometry3D::BoundsArrayType extended_bounds = bounds->GetGeometryForTimeStep(0)->GetBounds();
  for (unsigned int i = 0; i < 3; ++i)
  {
    if (bounds->GetExtentInWorld(i) < 500)
    {
      // extend the bounding box
      extended_bounds[i * 2] = center[i] - 500 / 2.0;
      extended_bounds[i * 2 + 1] = center[i] + 500 / 2.0;
    }
  }
  //set the extended bounds
  bounds->GetGeometryForTimeStep(0)->SetBounds(extended_bounds);

  // initialize the views to the bounding geometry
  mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
}
