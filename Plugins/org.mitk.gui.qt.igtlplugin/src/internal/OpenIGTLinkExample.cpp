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

// vtk
#include <vtkSphereSource.h>

//
#include "OpenIGTLinkExample.h"

//igtl
#include "igtlStringMessage.h"


const std::string OpenIGTLinkExample::VIEW_ID = "org.mitk.views.openigtlinkexample";

void OpenIGTLinkExample::SetFocus()
{
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
  m_IGTLClient->SetName("OIGTL Example Device");

  //create a new OpenIGTLinkExample Device source
  m_IGTLDeviceSource = mitk::IGTLDeviceSource::New();

  //set the client as the source for the device source
  m_IGTLDeviceSource->SetIGTLDevice(m_IGTLClient);

  m_IGTLDeviceSource->RegisterAsMicroservice();

  //create a filter that converts OpenIGTLinkExample messages into navigation data
  m_IGTLMsgToNavDataFilter = mitk::IGTLMessageToNavigationDataFilter::New();

  //create a visualization filter
  m_VisFilter = mitk::NavigationDataObjectVisualizationFilter::New();

  //connect the filters with each other
  //the OpenIGTLinkExample messages will be passed to the first filter that converts
  //it to navigation data, then it is passed to the visualization filter that
  //will visualize the transformation
  m_IGTLMsgToNavDataFilter->ConnectTo(m_IGTLDeviceSource);
  m_VisFilter->ConnectTo(m_IGTLMsgToNavDataFilter);

  //create an object that will be moved respectively to the navigation data
  m_DemoNode = mitk::DataNode::New();
  QString name =
      "IGTLDevice " + QString::fromStdString(m_IGTLClient->GetHostname());
  m_DemoNode->SetName(name.toStdString());

  //create small sphere and use it as surface
  mitk::Surface::Pointer mySphere = mitk::Surface::New();
  vtkSphereSource *vtkData = vtkSphereSource::New();
  vtkData->SetRadius(5.0f);
  vtkData->SetCenter(0.0, 0.0, 0.0);
  vtkData->Update();
  mySphere->SetVtkPolyData(vtkData->GetOutput());
  vtkData->Delete();
  m_DemoNode->SetData(mySphere);

  // add node to DataStorage
  this->GetDataStorage()->Add(m_DemoNode);

  //use this sphere as representation object
  m_VisFilter->SetRepresentationObject(0, mySphere);
}

void OpenIGTLinkExample::DestroyPipeline()
{
  m_VisFilter = NULL;
  this->GetDataStorage()->Remove(m_DemoNode);
}

void OpenIGTLinkExample::Start()
{
  m_Timer.setInterval(1000);
  m_Timer.start();
}

void OpenIGTLinkExample::UpdatePipeline()
{
  m_VisFilter->Update();

  //Update rendering
  QmitkRenderWindow* renWindow =
      this->GetRenderWindowPart()->GetQmitkRenderWindow("3d");
  renWindow->GetRenderer()->GetVtkRenderer()->Render();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
