/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkIGTLDeviceCommandWidget.h"

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

const std::string QmitkIGTLDeviceCommandWidget::VIEW_ID =
    "org.mitk.views.igtldevicesourcemanagementwidget";

QmitkIGTLDeviceCommandWidget::QmitkIGTLDeviceCommandWidget(
    QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f), m_IsClient(false), m_MessageReceivedObserverTag(0), m_CommandReceivedObserverTag(0), m_LostConnectionObserverTag(0), m_NewConnectionObserverTag(0), m_StateModifiedObserverTag(0)
{
  m_Controls = nullptr;
  this->m_IGTLDevice = nullptr;
  CreateQtPartControl(this);
}


QmitkIGTLDeviceCommandWidget::~QmitkIGTLDeviceCommandWidget()
{
   if (m_MessageReceivedObserverTag) this->m_IGTLDevice->RemoveObserver(m_MessageReceivedObserverTag);
   if (m_CommandReceivedObserverTag) this->m_IGTLDevice->RemoveObserver(m_CommandReceivedObserverTag);
   if (m_LostConnectionObserverTag) this->m_IGTLDevice->RemoveObserver(m_LostConnectionObserverTag);
   if (m_NewConnectionObserverTag) this->m_IGTLDevice->RemoveObserver(m_NewConnectionObserverTag);
   if (m_StateModifiedObserverTag) this->m_IGTLDevice->RemoveObserver(m_StateModifiedObserverTag);
}

void QmitkIGTLDeviceCommandWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkIGTLDeviceCommandWidgetControls;
    // setup GUI widgets
    m_Controls->setupUi(parent);
  }

  //connect slots with signals
  CreateConnections();
}

void QmitkIGTLDeviceCommandWidget::CreateConnections()
{
  if (m_Controls)
  {
    // connect the widget items with the methods
    connect( m_Controls->butSendCommand, SIGNAL(clicked()),
             this, SLOT(OnSendCommand()));
    connect( m_Controls->commandsComboBox,
             SIGNAL(currentIndexChanged(const QString &)),
             this, SLOT(OnCommandChanged(const QString &)));
  }
  //this is used for thread seperation, otherwise the worker thread would change the ui elements
  //which would cause an exception
  connect(this, SIGNAL(AdaptGUIToStateSignal()), this, SLOT(AdaptGUIToState()));
}


void QmitkIGTLDeviceCommandWidget::OnDeviceStateChanged()
{
   //this->AdaptGUIToState();
   emit AdaptGUIToStateSignal();
}

void QmitkIGTLDeviceCommandWidget::AdaptGUIToState()
{
  if (this->m_IGTLDevice.IsNotNull())
  {
    //check the state of the device
    mitk::IGTLDevice::IGTLDeviceState state = this->m_IGTLDevice->GetState();

    switch (state) {
    case mitk::IGTLDevice::Setup:
      this->m_Controls->commandsComboBox->setEnabled(false);
      this->m_Controls->butSendCommand->setEnabled(false);
      this->m_Controls->fpsSpinBox->setEnabled(false);
      break;
    case mitk::IGTLDevice::Ready:
      this->m_Controls->commandsComboBox->setEnabled(true);
      this->m_Controls->butSendCommand->setEnabled(true);
      this->m_Controls->fpsSpinBox->setEnabled(false);
      break;
    case mitk::IGTLDevice::Running:
      if ( this->m_IGTLDevice->GetNumberOfConnections() == 0 )
      {
        //just a server can run and have 0 connections
        this->m_Controls->butSendCommand->setEnabled(false);
        this->m_Controls->fpsSpinBox->setEnabled(false);
        this->m_Controls->commandsComboBox->setEnabled(false);
      }
      else
      {
        this->m_Controls->commandsComboBox->setEnabled(true);
        this->m_Controls->butSendCommand->setEnabled(true);
        //    this->m_Controls->fpsSpinBox->setEnabled(true);
      }
      break;
    default:
      mitkThrow() << "Invalid Device State";
      break;
    }
  }
  else
  {
    this->DisableSourceControls();
  }
}

void QmitkIGTLDeviceCommandWidget::Initialize(mitk::IGTLDevice::Pointer device)
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
    //get the device
    this->m_IGTLDevice = device;

    //check if the device is a server or a client
    if ( dynamic_cast<mitk::IGTLClient*>(
           this->m_IGTLDevice.GetPointer()) == nullptr )
    {
      m_IsClient = false;
    }
    else
    {
      m_IsClient = true;
    }

    typedef itk::SimpleMemberCommand< QmitkIGTLDeviceCommandWidget > CurCommandType;
//    CurCommandType::Pointer messageReceivedCommand = CurCommandType::New();
//    messageReceivedCommand->SetCallbackFunction(
//      this, &QmitkIGTLDeviceCommandWidget::OnMessageReceived );
//    this->m_MessageReceivedObserverTag =
//        this->m_IGTLDevice->AddObserver(mitk::MessageReceivedEvent(), messageReceivedCommand);

//    CurCommandType::Pointer commandReceivedCommand = CurCommandType::New();
//    commandReceivedCommand->SetCallbackFunction(
//      this, &QmitkIGTLDeviceCommandWidget::OnCommandReceived );
//    this->m_CommandReceivedObserverTag =
//        this->m_IGTLDevice->AddObserver(mitk::CommandReceivedEvent(), commandReceivedCommand);

    CurCommandType::Pointer connectionLostCommand = CurCommandType::New();
    connectionLostCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceCommandWidget::OnLostConnection );
    this->m_LostConnectionObserverTag = this->m_IGTLDevice->AddObserver(
          mitk::LostConnectionEvent(), connectionLostCommand);

    CurCommandType::Pointer newConnectionCommand = CurCommandType::New();
    newConnectionCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceCommandWidget::OnNewConnection );
    this->m_NewConnectionObserverTag = this->m_IGTLDevice->AddObserver(
          mitk::NewClientConnectionEvent(), newConnectionCommand);

    CurCommandType::Pointer stateModifiedCommand = CurCommandType::New();
    stateModifiedCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceCommandWidget::OnDeviceStateChanged );
    this->m_StateModifiedObserverTag = this->m_IGTLDevice->AddObserver(
          itk::ModifiedEvent(), stateModifiedCommand);

    //Fill the commands combo box with all available commands
    FillCommandsComboBox();
  }
  else
  {
    m_IGTLDevice = nullptr;
  }

  this->AdaptGUIToState();
}

void QmitkIGTLDeviceCommandWidget::DisableSourceControls()
{
  this->m_Controls->commandsComboBox->setEnabled(false);
  this->m_Controls->butSendCommand->setEnabled(false);
  this->m_Controls->fpsSpinBox->setEnabled(false);
}




void QmitkIGTLDeviceCommandWidget::OnSendCommand()
{
  //Set the frames per second of the current command in case of a STT_ command
  if ( std::strcmp( m_CurrentCommand->GetDeviceType(), "STT_BIND" ) == 0 )
  {
    ((igtl::StartBindMessage*)this->m_CurrentCommand.GetPointer())->
        SetResolution(this->m_Controls->fpsSpinBox->value());
  }
  else if ( std::strcmp( m_CurrentCommand->GetDeviceType(), "STT_QTDATA" ) == 0 )
  {
    ((igtl::StartQuaternionTrackingDataMessage*)m_CurrentCommand.GetPointer())->
        SetResolution(this->m_Controls->fpsSpinBox->value());
  }
  else if ( std::strcmp( m_CurrentCommand->GetDeviceType(), "STT_TDATA" ) == 0 )
  {
    ((igtl::StartTrackingDataMessage*)this->m_CurrentCommand.GetPointer())->
        SetResolution(this->m_Controls->fpsSpinBox->value());
  }

  m_IGTLDevice->SendMessage(mitk::IGTLMessage::New(m_CurrentCommand));
}

void QmitkIGTLDeviceCommandWidget::OnCommandChanged(
    const QString & curCommand)
{
  if ( curCommand.isEmpty() )
    return;

  mitk::IGTLMessageFactory::Pointer msgFactory =
      this->m_IGTLDevice->GetMessageFactory();
  //create a new message that fits to the selected get message type command
  this->m_CurrentCommand = msgFactory->CreateInstance( curCommand.toStdString());
  //enable/disable the FPS spinbox
  this->m_Controls->fpsSpinBox->setEnabled(curCommand.contains("STT_"));
}

void QmitkIGTLDeviceCommandWidget::OnLostConnection()
{
  //get the IGTL device that invoked this event
//  mitk::IGTLDevice* dev = (mitk::IGTLDevice*)caller;

  //this->AdaptGUIToState();
   emit AdaptGUIToStateSignal();
}

void QmitkIGTLDeviceCommandWidget::OnNewConnection()
{
   //this->AdaptGUIToState();
   emit AdaptGUIToStateSignal();
}

void QmitkIGTLDeviceCommandWidget::FillCommandsComboBox()
{
  //load the msg factory from the client (maybe this will be moved later on)
  mitk::IGTLMessageFactory::Pointer msgFactory =
      this->m_IGTLDevice->GetMessageFactory();
  //get the available commands as std::list<std::string>
  std::list<std::string> commandsList_ =
      msgFactory->GetAvailableMessageRequestTypes();
  //create a string list to convert the std::list
  this->m_Controls->commandsComboBox->clear();
  while ( commandsList_.size() )
  {
    //fill the combo box with life
    this->m_Controls->commandsComboBox->addItem(
          QString::fromStdString(commandsList_.front()));
    commandsList_.pop_front();
  }
}
