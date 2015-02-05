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

#include "QmitkIGTLDeviceSetupConnectionWidget.h"

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

const std::string QmitkIGTLDeviceSetupConnectionWidget::VIEW_ID =
    "org.mitk.views.igtldevicesetupconnectionwidget";

QmitkIGTLDeviceSetupConnectionWidget::QmitkIGTLDeviceSetupConnectionWidget(
    QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f), m_IsClient(false)
{
  m_Controls = NULL;
  this->m_IGTLDevice = NULL;
  CreateQtPartControl(this);
}


QmitkIGTLDeviceSetupConnectionWidget::~QmitkIGTLDeviceSetupConnectionWidget()
{
}

void QmitkIGTLDeviceSetupConnectionWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkIGTLDeviceSetupConnectionWidgetControls;
    // setup GUI widgets
    m_Controls->setupUi(parent);
  }

  // set the validator for the ip edit box (values must be between 0 and 255 and
  // there are four of them, seperated with a point
  QRegExpValidator *v = new QRegExpValidator(this);
  QRegExp rx("((1{0,1}[0-9]{0,2}|2[0-4]{1,1}[0-9]{1,1}|25[0-5]{1,1})\\.){3,3}(1{0,1}[0-9]{0,2}|2[0-4]{1,1}[0-9]{1,1}|25[0-5]{1,1})");
  v->setRegExp(rx);
  m_Controls->editIP->setValidator(v);
  // set the validator for the port edit box (values must be between 1 and 65535)
  m_Controls->editPort->setValidator(new QIntValidator(1, 65535, this));

  //connect slots with signals
  CreateConnections();
}

void QmitkIGTLDeviceSetupConnectionWidget::CreateConnections()
{
  if (m_Controls)
  {
    // connect the widget items with the methods
    connect( m_Controls->butConnect, SIGNAL(clicked()),
             this, SLOT(OnConnect()));
    connect( m_Controls->editPort, SIGNAL(editingFinished()),
             this, SLOT(OnPortChanged()) );
    connect( m_Controls->editIP, SIGNAL(editingFinished()),
             this, SLOT(OnHostnameChanged()) );
    connect( m_Controls->bufferMsgCheckBox, SIGNAL(stateChanged(int)),
             this, SLOT(OnBufferIncomingMessages(int)));
  }
}

void QmitkIGTLDeviceSetupConnectionWidget::OnDeviceStateChanged()
{
  this->AdaptGUIToState();
}

void QmitkIGTLDeviceSetupConnectionWidget::AdaptGUIToState()
{
  //check the validity of the device
  if ( this->m_IGTLDevice.IsNull() )
  {
    return;
  }

  //check the state of the device
  mitk::IGTLDevice::IGTLDeviceState state = this->m_IGTLDevice->GetState();

  switch (state) {
  case mitk::IGTLDevice::Setup:
    if ( !m_IsClient )
    {
      m_Controls->butConnect->setText("Go Online");
      this->m_Controls->editIP->setEnabled(false);
    }
    else
    {
      m_Controls->butConnect->setText("Connect");
      this->m_Controls->editIP->setEnabled(true);
    }
    this->m_Controls->editPort->setEnabled(true);
    this->m_Controls->logSendReceiveMsg->setEnabled(false);
    this->m_Controls->bufferMsgCheckBox->setEnabled(false);
    this->m_Controls->butConnect->setEnabled(true);
    break;
  case mitk::IGTLDevice::Ready:
    this->m_Controls->butConnect->setText("Disconnect");
    this->m_Controls->editIP->setEnabled(false);
    this->m_Controls->editPort->setEnabled(false);
    this->m_Controls->logSendReceiveMsg->setEnabled(true);
    this->m_Controls->bufferMsgCheckBox->setEnabled(true);
    this->m_Controls->butConnect->setEnabled(true);
    break;
  case mitk::IGTLDevice::Running:
    this->m_Controls->butConnect->setText("Disconnect");
    this->m_Controls->editIP->setEnabled(false);
    this->m_Controls->editPort->setEnabled(false);
    this->m_Controls->logSendReceiveMsg->setEnabled(true);
    this->m_Controls->bufferMsgCheckBox->setEnabled(true);
    this->m_Controls->butConnect->setEnabled(true);
    break;
  default:
    mitkThrow() << "Invalid Device State";
    break;
  }
}

void QmitkIGTLDeviceSetupConnectionWidget::Initialize(
    mitk::IGTLDevice::Pointer device)
{
  //reset the GUI
  DisableSourceControls();
  //reset the observers
  if ( this->m_IGTLDevice.IsNotNull() )
  {
    this->m_IGTLDevice->RemoveObserver(m_MessageReceivedObserverTag);
    this->m_IGTLDevice->RemoveObserver(m_CommandReceivedObserverTag);
    this->m_IGTLDevice->RemoveObserver(m_LostConnectionObserverTag);
    this->m_IGTLDevice->RemoveObserver(m_NewConnectionObserverTag);
    this->m_IGTLDevice->RemoveObserver(m_StateModifiedObserverTag);
  }

  if(device.IsNotNull())
  {
    this->m_IGTLDevice = device;

    //check if the device is a server or a client
    if ( dynamic_cast<mitk::IGTLClient*>(
           this->m_IGTLDevice.GetPointer()) == NULL )
    {
      m_IsClient = false;
    }
    else
    {
      m_IsClient = true;
    }

    this->AdaptGUIToState();

    typedef itk::SimpleMemberCommand< QmitkIGTLDeviceSetupConnectionWidget > CurCommandType;
    CurCommandType::Pointer messageReceivedCommand = CurCommandType::New();
    messageReceivedCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSetupConnectionWidget::OnMessageReceived );
    this->m_MessageReceivedObserverTag = this->m_IGTLDevice->AddObserver(
          mitk::MessageReceivedEvent(), messageReceivedCommand);

    CurCommandType::Pointer commandReceivedCommand = CurCommandType::New();
    commandReceivedCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSetupConnectionWidget::OnCommandReceived );
    this->m_CommandReceivedObserverTag = this->m_IGTLDevice->AddObserver(
          mitk::CommandReceivedEvent(), commandReceivedCommand);

    CurCommandType::Pointer connectionLostCommand = CurCommandType::New();
    connectionLostCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSetupConnectionWidget::OnLostConnection );
    this->m_LostConnectionObserverTag = this->m_IGTLDevice->AddObserver(
          mitk::LostConnectionEvent(), connectionLostCommand);

    CurCommandType::Pointer newConnectionCommand = CurCommandType::New();
    newConnectionCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSetupConnectionWidget::OnNewConnection );
    this->m_NewConnectionObserverTag = this->m_IGTLDevice->AddObserver(
          mitk::NewClientConnectionEvent(), newConnectionCommand);

    CurCommandType::Pointer stateModifiedCommand = CurCommandType::New();
    stateModifiedCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSetupConnectionWidget::OnDeviceStateChanged );
    this->m_StateModifiedObserverTag = this->m_IGTLDevice->AddObserver(
          itk::ModifiedEvent(), stateModifiedCommand);
  }
  else
  {
    m_IGTLDevice = NULL;
  }
}

void QmitkIGTLDeviceSetupConnectionWidget::DisableSourceControls()
{
  m_Controls->editIP->setEnabled(false);
  m_Controls->editPort->setEnabled(false);
  m_Controls->butConnect->setEnabled(false);
  m_Controls->bufferMsgCheckBox->setEnabled(false);
  m_Controls->logSendReceiveMsg->setEnabled(false);
}

void QmitkIGTLDeviceSetupConnectionWidget::OnConnect()
{
  if(m_Controls->butConnect->text() == "Connect" ||
     m_Controls->butConnect->text() == "Go Online" )
  {
    QString port = m_Controls->editPort->text();
    m_IGTLDevice->SetPortNumber(port.toInt());
    std::string hostname = m_Controls->editIP->text().toStdString();
    m_IGTLDevice->SetHostname(hostname);
    //connect with the other OpenIGTLink device => changes the state from Setup
    //to Ready
    if ( m_IGTLDevice->OpenConnection() )
    {
      //starts the communication thread => changes the state from Ready to
      //Running
      if ( m_IGTLDevice->StartCommunication() )
      {
        if ( this->m_IsClient )
        {
          MITK_INFO("IGTLDeviceSourceManagementWidget")
              << "Successfully connected to " << hostname
              << " on port " << port.toStdString();
        }
      }
      else
      {
        MITK_ERROR("QmitkIGTLDeviceSetupConnectionWidget") <<
                             "Could not start a communication with the"
                             "server because the client is in the wrong state";
      }
    }
    else
    {
      MITK_ERROR("QmitkIGTLDeviceSetupConnectionWidget") <<
                            "Could not connect to the server. "
                             "Please check the hostname and port.";
    }
  }
  else
  {
    m_IGTLDevice->CloseConnection();
    MITK_INFO("QmitkIGTLDeviceSetupConnectionWidget") << "Closed connection";
  }
  this->AdaptGUIToState();
}


void QmitkIGTLDeviceSetupConnectionWidget::OnPortChanged()
{

}


void QmitkIGTLDeviceSetupConnectionWidget::OnHostnameChanged()
{

}

void QmitkIGTLDeviceSetupConnectionWidget::OnLostConnection()
{
  //get the IGTL device that invoked this event
//  mitk::IGTLDevice* dev = (mitk::IGTLDevice*)caller;

  this->AdaptGUIToState();

//  unsigned int numConnections = dev->GetNumberOfConnections();

//  if ( numConnections == 0 )
//  {
//    if ( this->m_IsClient )
//    {
//      //Setup connection groupbox
//      m_Controls->editIP->setEnabled(true);
//      m_Controls->editPort->setEnabled(true);
//      m_Controls->butConnect->setText("Connect");
//      m_Controls->logSendReceiveMsg->setEnabled(false);
//      m_Controls->bufferMsgCheckBox->setEnabled(false);
//    }
//  }
}

void QmitkIGTLDeviceSetupConnectionWidget::OnNewConnection()
{
  this->AdaptGUIToState();

  //get the IGTL device that invoked this event
//  mitk::IGTLDevice* dev = (mitk::IGTLDevice*)caller;

//  unsigned int numConnections = dev->GetNumberOfConnections();

//  if ( numConnections != 0 )
//  {
//    //Setup connection groupbox
//    m_Controls->editIP->setEnabled(false);
//    m_Controls->editPort->setEnabled(false);
//    m_Controls->butConnect->setText("Disconnect");
//    m_Controls->logSendReceiveMsg->setEnabled(true);
//    m_Controls->bufferMsgCheckBox->setEnabled(true);
//  }
}

void QmitkIGTLDeviceSetupConnectionWidget::OnMessageReceived()
{
//  //get the IGTL device that invoked this event
//  mitk::IGTLDevice* dev = (mitk::IGTLDevice*)caller;

  if ( this->m_Controls->logSendReceiveMsg->isChecked() )
  {
    MITK_INFO("IGTLDeviceSetupConnectionWidget") << "Received a message: "
        << this->m_IGTLDevice->GetReceiveQueue()->GetLatestMsgInformationString();
  }
}

void QmitkIGTLDeviceSetupConnectionWidget::OnCommandReceived()
{
//  //get the IGTL device that invoked this event
//  mitk::IGTLDevice* dev = (mitk::IGTLDevice*)caller;

  if ( this->m_Controls->logSendReceiveMsg->isChecked() )
  {
    MITK_INFO("IGTLDeviceSetupConnectionWidget") << "Received a command: "
        << this->m_IGTLDevice->GetCommandQueue()->GetLatestMsgInformationString();
  }
}



void QmitkIGTLDeviceSetupConnectionWidget::OnBufferIncomingMessages(int state)
{
  if ( this->m_IGTLDevice )
  {
    this->m_IGTLDevice->EnableInfiniteBufferingMode(
          this->m_IGTLDevice->GetReceiveQueue(), (bool)state);
  }
}
