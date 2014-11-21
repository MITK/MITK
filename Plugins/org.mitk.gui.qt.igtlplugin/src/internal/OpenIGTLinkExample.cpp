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
  this->GetDataStorage()->Remove(m_DemoNodeT1);
  this->GetDataStorage()->Remove(m_DemoNodeT2);
  this->GetDataStorage()->Remove(m_DemoNodeT3);
}

void OpenIGTLinkExample::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  // connect the widget items with the methods
  connect( m_Controls.butStart, SIGNAL(clicked()),
           this, SLOT(Start()) );
  connect( &m_Timer, SIGNAL(timeout()), this, SLOT(UpdatePipeline()));

  //Setup the pipeline
  this->CreatePipeline();
}

void OpenIGTLinkExample::CreatePipeline()
{
  //create a new OpenIGTLinkExample Client
  m_IGTLClient = mitk::IGTLClient::New();
  m_IGTLClient->SetName("OIGTL Example Client Device");

  //create a new OpenIGTLinkExample Device source
  m_IGTLDeviceSource = mitk::IGTLDeviceSource::New();

  //set the client as the source for the device source
  m_IGTLDeviceSource->SetIGTLDevice(m_IGTLClient);

  m_IGTLDeviceSource->RegisterAsMicroservice();

  //create a filter that converts OpenIGTLinkExample messages into navigation data
  m_IGTLMsgToNavDataFilter = mitk::IGTLMessageToNavigationDataFilter::New();

  //create a visualization filter
  m_VisFilter = mitk::NavigationDataObjectVisualizationFilter::New();

  //we expect a tracking data message with three tools. Since we cannot change
  //the outputs at runtime we have to set it manually.
  m_IGTLMsgToNavDataFilter->SetNumberOfExpectedOutputs(3);

  //connect the filters with each other
  //the OpenIGTLinkExample messages will be passed to the first filter that converts
  //it to navigation data, then it is passed to the visualization filter that
  //will visualize the transformation
  m_IGTLMsgToNavDataFilter->ConnectTo(m_IGTLDeviceSource);
  m_VisFilter->ConnectTo(m_IGTLMsgToNavDataFilter);

  //create an object that will be moved respectively to the navigation data
  m_DemoNodeT1 = mitk::DataNode::New();
  m_DemoNodeT1->SetName("DemoNode IGTLExmpl T1");
  m_DemoNodeT2 = mitk::DataNode::New();
  m_DemoNodeT2->SetName("DemoNode IGTLExmpl T2");
  m_DemoNodeT3 = mitk::DataNode::New();
  m_DemoNodeT3->SetName("DemoNode IGTLExmpl T3");

  //create small sphere and use it as surface
  mitk::Surface::Pointer mySphere = mitk::Surface::New();
  vtkSphereSource *vtkData = vtkSphereSource::New();
  vtkData->SetRadius(2.0f);
  vtkData->SetCenter(0.0, 0.0, 0.0);
  vtkData->Update();
  mySphere->SetVtkPolyData(vtkData->GetOutput());
  vtkData->Delete();
  m_DemoNodeT1->SetData(mySphere);

  mitk::Surface::Pointer mySphere2 = mySphere->Clone();
  m_DemoNodeT2->SetData(mySphere2);
  mitk::Surface::Pointer mySphere3 = mySphere->Clone();
  m_DemoNodeT3->SetData(mySphere3);

  // add node to DataStorage
  this->GetDataStorage()->Add(m_DemoNodeT1);
  this->GetDataStorage()->Add(m_DemoNodeT2);
  this->GetDataStorage()->Add(m_DemoNodeT3);

  //use this sphere as representation object
  m_VisFilter->SetRepresentationObject(0, mySphere);
  m_VisFilter->SetRepresentationObject(1, mySphere2);
  m_VisFilter->SetRepresentationObject(2, mySphere3);
}

void OpenIGTLinkExample::DestroyPipeline()
{
  m_VisFilter = NULL;
  this->GetDataStorage()->Remove(m_DemoNodeT1);
  this->GetDataStorage()->Remove(m_DemoNodeT2);
  this->GetDataStorage()->Remove(m_DemoNodeT3);
}

void OpenIGTLinkExample::Start()
{
  if ( this->m_Controls.butStart->text().contains("Start Pipeline") )
  {
    m_Timer.setInterval(90);
    m_Timer.start();
    this->m_Controls.butStart->setText("Stop Pipeline");
  }
  else
  {
    m_Timer.stop();
    igtl::StopTrackingDataMessage::Pointer stopStreaming =
        igtl::StopTrackingDataMessage::New();
    this->m_IGTLClient->SendMessage(stopStreaming.GetPointer());
    this->m_Controls.butStart->setText("Start Pipeline");
  }
}

void OpenIGTLinkExample::UpdatePipeline()
{
  //update the pipeline
  m_VisFilter->Update();

  //update the boundings
  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());

  //Update rendering
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
