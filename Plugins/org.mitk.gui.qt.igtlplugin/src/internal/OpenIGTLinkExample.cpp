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
   this->DestroyPipeline();
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
  for (size_t i = 0; i < m_IGTLMsgToNavDataFilter->GetNumberOfIndexedOutputs(); i++)
  {
     mitk::DataNode::Pointer newNode = mitk::DataNode::New();
     QString name("DemoNode IGTLProviderExmpl T");
     name.append(QString::number(i));
     newNode->SetName(name.toStdString());

     //create small sphere and use it as surface
     mitk::Surface::Pointer mySphere = mitk::Surface::New();
     vtkSphereSource *vtkData = vtkSphereSource::New();
     vtkData->SetRadius(2.0f);
     vtkData->SetCenter(0.0, 0.0, 0.0);
     vtkData->Update();
     mySphere->SetVtkPolyData(vtkData->GetOutput());
     vtkData->Delete();
     newNode->SetData(mySphere);

     this->GetDataStorage()->Add(newNode);

     m_VisFilter->SetRepresentationObject(i, mySphere);

     m_DemoNodes.append(newNode);
  }
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
  if ( this->m_Controls.butStart->text().contains("Start Pipeline") )
  {
    m_Timer.setInterval(this->m_Controls.visualizationUpdateRateSpinBox->value());
    m_Timer.start();
    this->m_Controls.butStart->setText("Stop Pipeline");
    this->m_Controls.visualizationUpdateRateSpinBox->setEnabled(true);
  }
  else
  {
    m_Timer.stop();
    igtl::StopTrackingDataMessage::Pointer stopStreaming =
        igtl::StopTrackingDataMessage::New();
    this->m_IGTLClient->SendMessage(stopStreaming.GetPointer());
    this->m_Controls.butStart->setText("Start Pipeline");
    this->m_Controls.visualizationUpdateRateSpinBox->setEnabled(false);
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

  //check if the timer interval changed
  static int previousValue = 0;
  int currentValue = this->m_Controls.visualizationUpdateRateSpinBox->value();
  if (previousValue != currentValue)
  {
     m_Timer.setInterval(currentValue);
     previousValue = currentValue;
  }
}
