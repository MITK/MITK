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

#include "QmitkIGTLDeviceSourceManagementWidget.h"

//mitk headers
#include <mitkSurface.h>
#include <mitkIGTLDeviceSource.h>
#include <mitkDataStorage.h>
#include <mitkIGTLMessageFactory.h>

//qt headers
#include <qfiledialog.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qscrollbar.h>

//igtl
#include <igtlStringMessage.h>
#include <igtlBindMessage.h>
#include <igtlQuaternionTrackingDataMessage.h>
#include <igtlTrackingDataMessage.h>

//poco headers
#include <Poco/Path.h>

const std::string QmitkIGTLDeviceSourceManagementWidget::VIEW_ID =
    "org.mitk.views.igtldevicesourcemanagementwidget";

QmitkIGTLDeviceSourceManagementWidget::QmitkIGTLDeviceSourceManagementWidget(
    QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f), m_IsClient(false), m_MessageReceivedObserverTag(0), m_CommandReceivedObserverTag(0), m_LostConnectionObserverTag(0), m_NewConnectionObserverTag(0), m_StateModifiedObserverTag(0)
{
  m_Controls = NULL;
  this->m_IGTLDevice = NULL;
  CreateQtPartControl(this);
}


QmitkIGTLDeviceSourceManagementWidget::~QmitkIGTLDeviceSourceManagementWidget()
{
   if (m_MessageReceivedObserverTag) this->m_IGTLDevice->RemoveObserver(m_MessageReceivedObserverTag);
   if (m_CommandReceivedObserverTag) this->m_IGTLDevice->RemoveObserver(m_CommandReceivedObserverTag);
   if (m_LostConnectionObserverTag) this->m_IGTLDevice->RemoveObserver(m_LostConnectionObserverTag);
   if (m_NewConnectionObserverTag) this->m_IGTLDevice->RemoveObserver(m_NewConnectionObserverTag);
   if (m_StateModifiedObserverTag) this->m_IGTLDevice->RemoveObserver(m_StateModifiedObserverTag);
}

void QmitkIGTLDeviceSourceManagementWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkIGTLDeviceSourceManagementWidgetControls;
    // setup GUI widgets
    m_Controls->setupUi(parent);
  }

  //connect slots with signals
  CreateConnections();
}

void QmitkIGTLDeviceSourceManagementWidget::CreateConnections()
{
  if (m_Controls)
  {
    connect( m_Controls->butSend, SIGNAL(clicked()),
             this, SLOT(OnSendMessage()));
  }
  //this is used for thread seperation, otherwise the worker thread would change the ui elements
  //which would cause an exception
  connect(this, SIGNAL(AdaptGUIToStateSignal()), this, SLOT(AdaptGUIToState()));
}

void QmitkIGTLDeviceSourceManagementWidget::OnDeviceStateChanged()
{
   emit AdaptGUIToStateSignal();
}

void QmitkIGTLDeviceSourceManagementWidget::AdaptGUIToState()
{
  if (this->m_IGTLDeviceSource.IsNotNull())
  {
    //check the state of the device
    mitk::IGTLDevice::IGTLDeviceState state =
        this->m_IGTLDeviceSource->GetIGTLDevice()->GetState();

    switch (state) {
    case mitk::IGTLDevice::Setup:
      this->m_Controls->editSend->setEnabled(false);
      this->m_Controls->butSend->setEnabled(false);
      break;
    case mitk::IGTLDevice::Ready:
      this->m_Controls->editSend->setEnabled(false);
      this->m_Controls->butSend->setEnabled(false);
      break;
    case mitk::IGTLDevice::Running:
      if ( this->m_IGTLDevice->GetNumberOfConnections() == 0 )
      {
        //just a server can run and have 0 connections
        this->m_Controls->editSend->setEnabled(false);
        this->m_Controls->butSend->setEnabled(false);
      }
      else
      {
        this->m_Controls->editSend->setEnabled(true);
        this->m_Controls->butSend->setEnabled(true);
      }
      break;
    default:
      mitkThrow() << "Invalid Device State";
      break;
    }
    m_Controls->selectedSourceLabel->setText(
          m_IGTLDeviceSource->GetName().c_str());
  }
  else
  {
    this->DisableSourceControls();
  }
}

void QmitkIGTLDeviceSourceManagementWidget::LoadSource(
    mitk::IGTLDeviceSource::Pointer sourceToLoad)
{
  //reset the GUI
  DisableSourceControls();
  //reset the observers
  if ( this->m_IGTLDevice.IsNotNull() )
  {
    if (m_MessageReceivedObserverTag) this->m_IGTLDevice->RemoveObserver(m_MessageReceivedObserverTag);
    if (m_CommandReceivedObserverTag) this->m_IGTLDevice->RemoveObserver(m_CommandReceivedObserverTag);
    if (m_LostConnectionObserverTag) this->m_IGTLDevice->RemoveObserver(m_LostConnectionObserverTag);
    if (m_NewConnectionObserverTag) this->m_IGTLDevice->RemoveObserver(m_NewConnectionObserverTag);
    if (m_StateModifiedObserverTag) this->m_IGTLDevice->RemoveObserver(m_StateModifiedObserverTag);
  }

  if(sourceToLoad.IsNotNull())
  {
    this->m_IGTLDeviceSource = sourceToLoad;

    //get the device
    this->m_IGTLDevice = this->m_IGTLDeviceSource->GetIGTLDevice();

    //initialize the other GUI elements
    this->m_Controls->connectionSetupWidget->Initialize(this->m_IGTLDevice);
    this->m_Controls->commandWidget->Initialize(this->m_IGTLDevice);

    //check if the device is a server or a client
    if ( dynamic_cast<mitk::IGTLClient*>(
           this->m_IGTLDeviceSource->GetIGTLDevice()) == NULL )
    {
      m_IsClient = false;
    }
    else
    {
      m_IsClient = true;
    }

    typedef itk::SimpleMemberCommand< QmitkIGTLDeviceSourceManagementWidget > CurCommandType;
    CurCommandType::Pointer messageReceivedCommand = CurCommandType::New();
    messageReceivedCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSourceManagementWidget::OnMessageReceived );
    this->m_MessageReceivedObserverTag =
        this->m_IGTLDevice->AddObserver(mitk::MessageReceivedEvent(), messageReceivedCommand);

    CurCommandType::Pointer commandReceivedCommand = CurCommandType::New();
    commandReceivedCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSourceManagementWidget::OnCommandReceived );
    this->m_CommandReceivedObserverTag =
        this->m_IGTLDevice->AddObserver(mitk::CommandReceivedEvent(), commandReceivedCommand);

    CurCommandType::Pointer connectionLostCommand = CurCommandType::New();
    connectionLostCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSourceManagementWidget::OnLostConnection );
    this->m_LostConnectionObserverTag = this->m_IGTLDevice->AddObserver(
          mitk::LostConnectionEvent(), connectionLostCommand);

    CurCommandType::Pointer newConnectionCommand = CurCommandType::New();
    newConnectionCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSourceManagementWidget::OnNewConnection );
    this->m_NewConnectionObserverTag = this->m_IGTLDevice->AddObserver(
          mitk::NewClientConnectionEvent(), newConnectionCommand);

    CurCommandType::Pointer stateModifiedCommand = CurCommandType::New();
    stateModifiedCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSourceManagementWidget::OnDeviceStateChanged );
    this->m_StateModifiedObserverTag = this->m_IGTLDevice->AddObserver(
          itk::ModifiedEvent(), stateModifiedCommand);
  }
  else
  {
    m_IGTLDeviceSource = NULL;
  }
  this->AdaptGUIToState();
}

void QmitkIGTLDeviceSourceManagementWidget::DisableSourceControls()
{
  m_Controls->selectedSourceLabel->setText("<none>");
  m_Controls->editSend->setEnabled(false);
  m_Controls->butSend->setEnabled(false);
}


void QmitkIGTLDeviceSourceManagementWidget::OnSendMessage()
{
  std::string toBeSend = m_Controls->editSend->text().toStdString();

  igtl::StringMessage::Pointer msg = igtl::StringMessage::New();
  msg->SetString(toBeSend);
  this->m_IGTLDevice->SendMessage(msg.GetPointer());
}

void QmitkIGTLDeviceSourceManagementWidget::OnMessageReceived()
{

}

void QmitkIGTLDeviceSourceManagementWidget::OnCommandReceived()
{

}

void QmitkIGTLDeviceSourceManagementWidget::OnLostConnection()
{
   emit AdaptGUIToStateSignal();
}

void QmitkIGTLDeviceSourceManagementWidget::OnNewConnection()
{
  emit AdaptGUIToStateSignal();
}
