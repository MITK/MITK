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

// mitk
#include <mitkImage.h>
#include <mitkSurface.h>
#include <mitkIGTLMessageToNavigationDataFilter.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

// vtk
#include <vtkSphereSource.h>

//
#include "OpenIGTLinkExample.h"

//igtl
#include "igtlStringMessage.h"
#include "igtlTrackingDataMessage.h"

const std::string OpenIGTLinkExample::VIEW_ID = "org.mitk.views.OpenIGTLinkExample";

void OpenIGTLinkExample::SetFocus()
{
}

OpenIGTLinkExample::~OpenIGTLinkExample()
{
   this->DestroyPipeline();

   if (m_IGTLDeviceSource.IsNotNull())
   {
      m_IGTLDeviceSource->UnRegisterMicroservice();
   }
}

void OpenIGTLinkExample::CreateQtPartControl( QWidget *parent )
{

  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  // connect the widget items with the methods
  connect( m_Controls.butStart, SIGNAL(clicked()),
           this, SLOT(Start()) );
  connect( &m_Timer, SIGNAL(timeout()), this, SLOT(UpdatePipeline()));

  //create a new OpenIGTLinkExample Client
  m_IGTLClient = mitk::IGTLClient::New(false);
  m_IGTLClient->SetName("OIGTL Example Client Device");

  //create a new OpenIGTLinkExample Device source
  m_IGTLDeviceSource = mitk::IGTLDeviceSource::New();

  //set the client as the source for the device source
  m_IGTLDeviceSource->SetIGTLDevice(m_IGTLClient);

  m_IGTLDeviceSource->RegisterAsMicroservice();
}

void OpenIGTLinkExample::CreatePipeline()
{
  //create a filter that converts OpenIGTLinkExample messages into navigation data
  m_IGTLMsgToNavDataFilter = mitk::IGTLMessageToNavigationDataFilter::New();

  //create a visualization filter
  m_VisFilter = mitk::NavigationDataObjectVisualizationFilter::New();

  //we expect a tracking data message with three tools. Since we cannot change
  //the outputs at runtime we have to set it manually.
  m_IGTLMsgToNavDataFilter->SetNumberOfExpectedOutputs(m_Controls.channelSpinBox->value());

  //connect the filters with each other
  //the OpenIGTLinkExample messages will be passed to the first filter that converts
  //it to navigation data, then it is passed to the visualization filter that
  //will visualize the transformation
  m_IGTLMsgToNavDataFilter->ConnectTo(m_IGTLDeviceSource);
  m_VisFilter->ConnectTo(m_IGTLMsgToNavDataFilter);

  //create an object that will be moved respectively to the navigation data
  for (size_t i = 0; i < m_IGTLMsgToNavDataFilter->GetNumberOfIndexedOutputs(); i++)
  {
     mitk::DataNode::Pointer newNode = mitk::DataNode::New();
     QString name("DemoNode IGTLProviderExmpl T");
     name.append(QString::number(i));
     newNode->SetName(name.toStdString());

     //create small sphere and use it as surface
     mitk::Surface::Pointer mySphere = mitk::Surface::New();
     vtkSmartPointer<vtkSphereSource> vtkSphere = vtkSmartPointer<vtkSphereSource>::New();
     vtkSphere->SetRadius(2.0f);
     vtkSphere->SetCenter(0.0, 0.0, 0.0);
     vtkSphere->Update();
     mySphere->SetVtkPolyData(vtkSphere->GetOutput());
     newNode->SetData(mySphere);

     m_VisFilter->SetRepresentationObject(i, mySphere.GetPointer());

     m_DemoNodes.append(newNode);
  }

  this->ResizeBoundingBox();
}

void OpenIGTLinkExample::DestroyPipeline()
{
  m_VisFilter = nullptr;
  foreach(mitk::DataNode::Pointer node, m_DemoNodes)
  {
     this->GetDataStorage()->Remove(node);
  }
  this->m_DemoNodes.clear();
}

void OpenIGTLinkExample::Start()
{
  if (this->m_Controls.butStart->text().contains("Start Pipeline"))
  {
    static bool isFirstTime = true;
    if (isFirstTime)
    {
      //Setup the pipeline
      this->CreatePipeline();
      isFirstTime = false;
    }

    m_Timer.setInterval(this->m_Controls.visualizationUpdateRateSpinBox->value());
    m_Timer.start();
    //this->m_Controls.visualizationUpdateRateSpinBox->setEnabled(true);
    this->m_Controls.butStart->setText("Stop Pipeline");
  }
  else
  {
    m_Timer.stop();
    igtl::StopTrackingDataMessage::Pointer stopStreaming =
      igtl::StopTrackingDataMessage::New();
    this->m_IGTLClient->SendMessage(mitk::IGTLMessage::New((igtl::MessageBase::Pointer) stopStreaming));
    this->m_Controls.butStart->setText("Start Pipeline");
    //this->m_Controls.visualizationUpdateRateSpinBox->setEnabled(false);
  }
}

void OpenIGTLinkExample::UpdatePipeline()
{
  if (this->m_Controls.visualizeCheckBox->isChecked())
  {
    //update the pipeline
    m_VisFilter->Update();

    ////update the boundings
    //mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());

    //Update rendering
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else
  {
    //no visualization so we just update this filter
    m_IGTLMsgToNavDataFilter->Update();
    //record a timestamp if the output is new
    //static double previousTimestamp;
    //double curTimestamp = m_IGTLMsgToNavDataFilter->GetOutput()->GetIGTTimeStamp();
    //if (previousTimestamp != curTimestamp)
    static mitk::NavigationData::Pointer previousND = mitk::NavigationData::New();
    mitk::NavigationData* curND = m_IGTLMsgToNavDataFilter->GetOutput();

    //std::cout << "9: igt timestamp: " << curND->GetIGTTimeStamp() << std::endl;
    //std::cout << "9: timestamp: " << curND->GetTimeStamp() << std::endl;

    if ( !mitk::Equal( *(previousND.GetPointer()), *curND ) )
    {
      //previousTimestamp = curTimestamp;
      previousND->Graft(curND);
    }
  }

  //check if the timer interval changed
  static int previousValue = 0;
  int currentValue = this->m_Controls.visualizationUpdateRateSpinBox->value();
  if (previousValue != currentValue)
  {
    m_Timer.setInterval(currentValue);
    previousValue = currentValue;
  }
}

/**
* \brief To initialize the scene to the bounding box of all visible objects
*/
void OpenIGTLinkExample::ResizeBoundingBox()
{
  // get all nodes
  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetDataStorage()->GetAll();
  auto bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(rs);

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
      extended_bounds[i * 2]     = center[i] - 500 / 2.0;
      extended_bounds[i * 2 + 1] = center[i] + 500 / 2.0;
    }
  }
  //set the extended bounds
  bounds->GetGeometryForTimeStep(0)->SetBounds(extended_bounds);

  // initialize the views to the bounding geometry
  mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
}
