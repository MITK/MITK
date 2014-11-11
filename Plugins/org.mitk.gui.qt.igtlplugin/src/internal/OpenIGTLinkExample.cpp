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


const std::string OpenIGTLinkExample::VIEW_ID = "org.mitk.views.openigtlink";

void OpenIGTLinkExample::SetFocus()
{
//  m_Controls.buttonPerformImageProcessing->setFocus();
}

void OpenIGTLinkExample::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  // connect the widget items with the methods
  connect( m_Controls.butConnectWithServer, SIGNAL(clicked()),
           this, SLOT(ConnectWithServer()));
  connect( m_Controls.editPort, SIGNAL(editingFinished()),
           this, SLOT(ChangePort()) );
  connect( m_Controls.editIP, SIGNAL(editingFinished()),
           this, SLOT(ChangeIP()) );
  connect( m_Controls.butStart, SIGNAL(clicked()),
           this, SLOT(Start()) );
  connect( &m_Timer, SIGNAL(timeout()), this, SLOT(UpdatePipeline()));
  connect( m_Controls.butSend, SIGNAL(clicked()), this, SLOT(SendMessage()));

  // set the validator for the ip edit box (values must be between 0 and 255 and
  // there are four of them, seperated with a point
  QRegExpValidator *v = new QRegExpValidator(this);
  QRegExp rx("((1{0,1}[0-9]{0,2}|2[0-4]{1,1}[0-9]{1,1}|25[0-5]{1,1})\\.){3,3}(1{0,1}[0-9]{0,2}|2[0-4]{1,1}[0-9]{1,1}|25[0-5]{1,1})");
  v->setRegExp(rx);
  m_Controls.editIP->setValidator(v);
  // set the validator for the port edit box (values must be between 1 and 65535)
  m_Controls.editPort->setValidator(new QIntValidator(1, 65535, this));

  //Setup the pipeline
  this->CreatePipeline();

  //update the pipeline
//  m_VisFilter->Update();
}

void OpenIGTLinkExample::ConnectWithServer()
{
    if(m_Controls.butConnectWithServer->text() == "Connect")
    {
        m_IGTLClient->SetPortNumber(m_Controls.editPort->text().toInt());
        m_IGTLClient->SetHostname(m_Controls.editIP->text().toStdString());
        m_IGTLClient->OpenConnection();
        if ( m_IGTLClient->StartCommunication() )
        {
          m_Controls.editIP->setEnabled(false);
          m_Controls.editPort->setEnabled(false);
          m_Controls.editSend->setEnabled(true);
          m_Controls.butConnectWithServer->setText("Disconnect");
        }
        else
        {
          MITK_ERROR("OpenIGTLinkExample") << "Could not start a communication with the"
          "server. Please check the hostname and port.";
        }
//        Start();
    }
    else
    {
        m_Controls.editIP->setEnabled(true);
        m_Controls.editPort->setEnabled(true);
        m_Controls.editSend->setEnabled(false);
        m_Controls.butConnectWithServer->setText("Connect");
        m_IGTLClient->CloseConnection();
    }
}

void OpenIGTLinkExample::ChangePort()
{

}

void OpenIGTLinkExample::ChangeIP()
{

}

void OpenIGTLinkExample::CreatePipeline()
{
  //create a new OpenIGTLinkExample Client
  m_IGTLClient = mitk::IGTLClient::New();

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

void OpenIGTLinkExample::SendMessage()
{
  std::string toBeSend = m_Controls.editSend->text().toStdString();

  igtl::StringMessage::Pointer msg = igtl::StringMessage::New().GetPointer();
  msg->SetString(toBeSend);
  if ( m_IGTLClient->SendMessage(msg.GetPointer()) )
  {
    MITK_INFO("OpenIGTLinkExample") << "Successfully sent the message.";
  }
  else
  {
    MITK_ERROR("OpenIGTLinkExample") << "Could not send the message.";
  }
}
