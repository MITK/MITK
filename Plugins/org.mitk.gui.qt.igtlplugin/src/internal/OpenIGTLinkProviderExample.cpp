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
#include <QFileDialog>

// mitk
#include <mitkImage.h>
#include <mitkSurface.h>
#include <mitkIGTLMessageToNavigationDataFilter.h>
#include <mitkNavigationDataReaderXML.h>

// vtk
#include <vtkSphereSource.h>

//
#include "OpenIGTLinkProviderExample.h"

//igtl
#include "igtlStringMessage.h"


const std::string OpenIGTLinkProviderExample::VIEW_ID =
    "org.mitk.views.OpenIGTLinkProviderExample";

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
}

void OpenIGTLinkProviderExample::CreatePipeline()
{
  //create a new OpenIGTLink Client
  m_IGTLServer = mitk::IGTLServer::New();
  m_IGTLServer->SetName("OIGTL Provider Example Device");

  //create a new OpenIGTLink Device source
  m_IGTLMessageProvider = mitk::IGTLMessageProvider::New();

  //set the client as the source for the device source
  m_IGTLMessageProvider->SetIGTLDevice(m_IGTLServer);

  //register the provider so that it can be configured with the IGTL manager
  //plugin. This could be hardcoded but now I already have the fancy plugin.
  m_IGTLMessageProvider->RegisterAsMicroservice();

  //create a filter that converts navigation data into IGTL messages
  m_NavDataToIGTLMsgFilter = mitk::NavigationDataToIGTLMessageFilter::New();

  //register this filter as micro service. The message provider looks for
  //provided IGTLMessageSources, once it found this microservice and someone
  //requested this data type then the provider will connect with this filter
  //automatically
  m_NavDataToIGTLMsgFilter->RegisterAsMicroservice();

  //define the operation mode for this filter
  m_NavDataToIGTLMsgFilter->SetOperationMode(
        mitk::NavigationDataToIGTLMessageFilter::ModeSendQTransMsg);

  //create a navigation data player object that will play nav data from a
  //recorded file
  m_NavDataPlayer = mitk::NavigationDataPlayer::New();

  //connect the filters with each other
  //the navigation data player reads a file with recorded navigation data,
  //passes this data to a filter that converts it into a IGTLMessage.
  //The provider is not connected because it will search for fitting services.
  //Once it found the filter it will automatically connect to it.
  m_NavDataToIGTLMsgFilter->ConnectTo(m_NavDataPlayer);

  //create an object that will be moved respectively to the navigation data
  m_DemoNode = mitk::DataNode::New();
  QString name =
      "IGTLDevice " + QString::fromStdString(m_IGTLServer->GetHostname());
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
  m_NavDataPlayer->SetNavigationDataSet(m_NavDataSet);
}

void OpenIGTLinkProviderExample::DestroyPipeline()
{
  m_NavDataPlayer->StopPlaying();
  this->GetDataStorage()->Remove(m_DemoNode);
}

void OpenIGTLinkProviderExample::Start()
{
  if ( this->m_Controls.butStart->text().contains("Start") )
  {
    m_NavDataPlayer->StartPlaying();
    this->m_Controls.butStart->setText("Stop Playing Recorded Navigation Data ");
  }
  else
  {
    m_NavDataPlayer->StopPlaying();
    this->m_Controls.butStart->setText("Start Playing Recorded Navigation Data ");
  }
}

void OpenIGTLinkProviderExample::OnOpenFile(){
  mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();

  // FIXME Filter for correct files and use correct Reader
  QString fileName =
      QFileDialog::getOpenFileName(NULL, "Open Navigation Data Set", "", "XML files (*.xml)");
  if ( fileName.isNull() ) { return; } // user pressed cancel

  try
  {
    m_NavDataSet = reader->Read(fileName.toStdString());
  }
  catch ( const mitk::Exception &e )
  {
    MITK_WARN("NavigationDataPlayerView") << "could not open file " << fileName.toStdString();
    QMessageBox::critical(0, "Error Reading File", "The file '" + fileName
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
