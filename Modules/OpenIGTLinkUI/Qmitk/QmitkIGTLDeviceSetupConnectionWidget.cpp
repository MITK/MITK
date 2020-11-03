/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

#include <QRegExpValidator>

const std::string QmitkIGTLDeviceSetupConnectionWidget::VIEW_ID =
"org.mitk.views.igtldevicesetupconnectionwidget";

QmitkIGTLDeviceSetupConnectionWidget::QmitkIGTLDeviceSetupConnectionWidget(
  QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f), m_IsClient(false)
{
  m_Controls = nullptr;
  this->m_IGTLDevice = nullptr;
  CreateQtPartControl(this);
  m_NumSentFramesSinceLastUpdate = 0;
  m_NumReceivedFramesSinceLastUpdate = 0;
}

QmitkIGTLDeviceSetupConnectionWidget::~QmitkIGTLDeviceSetupConnectionWidget()
{
  this->RemoveObserver();
}

void QmitkIGTLDeviceSetupConnectionWidget::RemoveObserver()
{
  if (this->m_IGTLDevice.IsNotNull())
  {
    this->m_IGTLDevice->RemoveObserver(m_MessageReceivedObserverTag);
    this->m_IGTLDevice->RemoveObserver(m_MessageSentObserverTag);
    this->m_IGTLDevice->RemoveObserver(m_CommandReceivedObserverTag);
    this->m_IGTLDevice->RemoveObserver(m_LostConnectionObserverTag);
    this->m_IGTLDevice->RemoveObserver(m_NewConnectionObserverTag);
    this->m_IGTLDevice->RemoveObserver(m_StateModifiedObserverTag);
  }
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

  m_FPSCalculationTimer.start(1000);

  //connect slots with signals
  CreateConnections();
}

void QmitkIGTLDeviceSetupConnectionWidget::CreateConnections()
{
  if (m_Controls)
  {
    // connect the widget items with the methods
    connect(m_Controls->butConnect, SIGNAL(clicked()),
      this, SLOT(OnConnect()));
    connect(m_Controls->editPort, SIGNAL(editingFinished()),
      this, SLOT(OnPortChanged()));
    connect(m_Controls->editIP, SIGNAL(editingFinished()),
      this, SLOT(OnHostnameChanged()));
    connect(m_Controls->bufferInMsgCheckBox, SIGNAL(stateChanged(int)),
      this, SLOT(OnBufferIncomingMessages(int)));
    connect(m_Controls->bufferOutMsgCheckBox, SIGNAL(stateChanged(int)),
      this, SLOT(OnBufferOutgoingMessages(int)));
    connect(&m_FPSCalculationTimer, SIGNAL(timeout()),
      this, SLOT(OnUpdateFPSLabel()));
    connect(m_Controls->logMessageDetailsCheckBox, SIGNAL(clicked()),
      this, SLOT(OnLogMessageDetailsCheckBoxClicked()));
  }
  //this is used for thread seperation, otherwise the worker thread would change the ui elements
  //which would cause an exception
  connect(this, SIGNAL(AdaptGUIToStateSignal()), this, SLOT(AdaptGUIToState()));
}

void QmitkIGTLDeviceSetupConnectionWidget::OnDeviceStateChanged()
{
  emit AdaptGUIToStateSignal();
}

void QmitkIGTLDeviceSetupConnectionWidget::AdaptGUIToState()
{
  //check the validity of the device
  if (this->m_IGTLDevice.IsNull())
  {
    return;
  }

  //check the state of the device
  mitk::IGTLDevice::IGTLDeviceState state = this->m_IGTLDevice->GetState();

  switch (state) {
  case mitk::IGTLDevice::Setup:
    if (!m_IsClient)
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
    this->m_Controls->logMessageStatusCheckBox->setChecked(false);
    this->m_Controls->logMessageDetailsCheckBox->setChecked(false);
    this->m_Controls->logMessageStatusCheckBox->setEnabled(false);
    this->m_Controls->logMessageDetailsCheckBox->setEnabled(false);
    this->m_Controls->bufferInMsgCheckBox->setEnabled(false);
    this->m_Controls->bufferOutMsgCheckBox->setEnabled(false);
    this->m_Controls->butConnect->setEnabled(true);
    this->m_Controls->fpsInLabel->setEnabled(false);
    this->m_Controls->fpsOutLabel->setEnabled(false);
    this->m_Controls->fpsInDescrLabel->setEnabled(false);
    this->m_Controls->fpsOutDescrLabel->setEnabled(false);

    if( this->m_IGTLDevice.IsNotNull() )
    {
      this->m_IGTLDevice->SetLogMessages(false);
    }

    break;
  case mitk::IGTLDevice::Ready:
    if (m_IsClient)
    {
      this->m_Controls->butConnect->setText("Disconnect");
    }
    else
    {
      this->m_Controls->butConnect->setText("Go Offline");
    }
    this->m_Controls->editIP->setEnabled(false);
    this->m_Controls->editPort->setEnabled(false);
    this->m_Controls->logMessageStatusCheckBox->setEnabled(true);
    this->m_Controls->logMessageDetailsCheckBox->setEnabled(true);
    this->m_Controls->bufferInMsgCheckBox->setEnabled(true);
    this->m_Controls->bufferOutMsgCheckBox->setEnabled(true);
    this->m_Controls->butConnect->setEnabled(true);
    this->m_Controls->fpsInLabel->setEnabled(true);
    this->m_Controls->fpsOutLabel->setEnabled(true);
    this->m_Controls->fpsInDescrLabel->setEnabled(true);
    this->m_Controls->fpsOutDescrLabel->setEnabled(true);
    break;
  case mitk::IGTLDevice::Running:
    if (m_IsClient)
    {
      this->m_Controls->butConnect->setText("Disconnect");
    }
    else
    {
      this->m_Controls->butConnect->setText("Go Offline");
    }
    this->m_Controls->editIP->setEnabled(false);
    this->m_Controls->editPort->setEnabled(false);
    this->m_Controls->logMessageStatusCheckBox->setEnabled(true);
    this->m_Controls->logMessageDetailsCheckBox->setEnabled(true);
    this->m_Controls->bufferInMsgCheckBox->setEnabled(true);
    this->m_Controls->bufferOutMsgCheckBox->setEnabled(true);
    this->m_Controls->butConnect->setEnabled(true);
    this->m_Controls->fpsInLabel->setEnabled(true);
    this->m_Controls->fpsOutLabel->setEnabled(true);
    this->m_Controls->fpsInDescrLabel->setEnabled(true);
    this->m_Controls->fpsOutDescrLabel->setEnabled(true);
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
  this->RemoveObserver();

  if (device.IsNotNull())
  {
    this->m_IGTLDevice = device;

    //check if the device is a server or a client
    if (dynamic_cast<mitk::IGTLClient*>(
      this->m_IGTLDevice.GetPointer()) == nullptr)
    {
      m_IsClient = false;
    }
    else
    {
      m_IsClient = true;
    }

    this->AdaptGUIToState();

    typedef itk::SimpleMemberCommand< QmitkIGTLDeviceSetupConnectionWidget > CurCommandType;
    CurCommandType::Pointer messageSentCommand = CurCommandType::New();
    messageSentCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSetupConnectionWidget::OnMessageSent);
    this->m_MessageSentObserverTag = this->m_IGTLDevice->AddObserver(
      mitk::MessageSentEvent(), messageSentCommand);

    CurCommandType::Pointer messageReceivedCommand = CurCommandType::New();
    messageReceivedCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSetupConnectionWidget::OnMessageReceived);
    this->m_MessageReceivedObserverTag = this->m_IGTLDevice->AddObserver(
      mitk::MessageReceivedEvent(), messageReceivedCommand);

    CurCommandType::Pointer commandReceivedCommand = CurCommandType::New();
    commandReceivedCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSetupConnectionWidget::OnCommandReceived);
    this->m_CommandReceivedObserverTag = this->m_IGTLDevice->AddObserver(
      mitk::CommandReceivedEvent(), commandReceivedCommand);

    CurCommandType::Pointer connectionLostCommand = CurCommandType::New();
    connectionLostCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSetupConnectionWidget::OnLostConnection);
    this->m_LostConnectionObserverTag = this->m_IGTLDevice->AddObserver(
      mitk::LostConnectionEvent(), connectionLostCommand);

    CurCommandType::Pointer newConnectionCommand = CurCommandType::New();
    newConnectionCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSetupConnectionWidget::OnNewConnection);
    this->m_NewConnectionObserverTag = this->m_IGTLDevice->AddObserver(
      mitk::NewClientConnectionEvent(), newConnectionCommand);

    CurCommandType::Pointer stateModifiedCommand = CurCommandType::New();
    stateModifiedCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSetupConnectionWidget::OnDeviceStateChanged);
    this->m_StateModifiedObserverTag = this->m_IGTLDevice->AddObserver(
      itk::ModifiedEvent(), stateModifiedCommand);

    OnBufferIncomingMessages(m_Controls->bufferInMsgCheckBox->isChecked());
    OnBufferOutgoingMessages(m_Controls->bufferOutMsgCheckBox->isChecked());
  }
  else
  {
    m_IGTLDevice = nullptr;
  }
}

void QmitkIGTLDeviceSetupConnectionWidget::DisableSourceControls()
{
  m_Controls->editIP->setEnabled(false);
  m_Controls->editPort->setEnabled(false);
  m_Controls->butConnect->setEnabled(false);
  m_Controls->bufferInMsgCheckBox->setEnabled(false);
  m_Controls->bufferOutMsgCheckBox->setEnabled(false);
  this->m_Controls->logMessageStatusCheckBox->setChecked(false);
  this->m_Controls->logMessageDetailsCheckBox->setChecked(false);
  this->m_Controls->logMessageStatusCheckBox->setEnabled(false);
  this->m_Controls->logMessageDetailsCheckBox->setEnabled(false);

  if( this->m_IGTLDevice.IsNotNull() )
  {
    this->m_IGTLDevice->SetLogMessages(false);
  }
}

void QmitkIGTLDeviceSetupConnectionWidget::OnConnect()
{
  if (m_IGTLDevice->GetState() == mitk::IGTLDevice::Setup)
  {
    QString port = m_Controls->editPort->text();
    m_IGTLDevice->SetPortNumber(port.toInt());
    std::string hostname = m_Controls->editIP->text().toStdString();
    m_IGTLDevice->SetHostname(hostname);
    //connect with the other OpenIGTLink device => changes the state from Setup
    //to Ready
    if (m_IGTLDevice->OpenConnection())
    {
      //starts the communication thread => changes the state from Ready to
      //Running
      if (m_IGTLDevice->StartCommunication())
      {
        if (this->m_IsClient)
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
  else if (m_IGTLDevice->GetState() == mitk::IGTLDevice::Ready || m_IGTLDevice->GetState() == mitk::IGTLDevice::Running)
  {
    m_IGTLDevice->CloseConnection();
    MITK_INFO("QmitkIGTLDeviceSetupConnectionWidget") << "Closed connection";
  }
  else
  {
    mitkThrow() << "Invalid state of IGTLDevice";
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
  emit AdaptGUIToStateSignal();
}

void QmitkIGTLDeviceSetupConnectionWidget::OnNewConnection()
{
  emit AdaptGUIToStateSignal();
}

void QmitkIGTLDeviceSetupConnectionWidget::OnMessageReceived()
{
  if( this->m_Controls->logMessageStatusCheckBox->isChecked() )
  {
    MITK_INFO("IGTLDeviceSetupConnectionWidget") << "Received a message.";
  }
  m_NumReceivedFramesSinceLastUpdate++;
}

void QmitkIGTLDeviceSetupConnectionWidget::OnMessageSent()
{
  if( this->m_Controls->logMessageStatusCheckBox->isChecked() )
  {
    MITK_INFO("IGTLDeviceSetupConnectionWidget") << "Sent a message.";
  }
  m_NumSentFramesSinceLastUpdate++;
}

void QmitkIGTLDeviceSetupConnectionWidget::OnCommandReceived()
{
  if( this->m_Controls->logMessageStatusCheckBox->isChecked() )
  {
    MITK_INFO("IGTLDeviceSetupConnectionWidget") << "Received a command.";
  }
}

void QmitkIGTLDeviceSetupConnectionWidget::OnBufferIncomingMessages(int state)
{
  if (this->m_IGTLDevice.IsNotNull())
  {
    this->m_IGTLDevice->EnableNoBufferingMode(
      this->m_IGTLDevice->GetMessageQueue(), (bool)state);
  }
}

void QmitkIGTLDeviceSetupConnectionWidget::OnBufferOutgoingMessages(int state)
{
  if (this->m_IGTLDevice.IsNotNull())
  {
    this->m_IGTLDevice->EnableNoBufferingMode(
      this->m_IGTLDevice->GetMessageQueue(), (bool)state);
  }
}

void QmitkIGTLDeviceSetupConnectionWidget::OnUpdateFPSLabel()
{
  double fpsIn = m_NumReceivedFramesSinceLastUpdate / 1.0;
  double fpsOut = m_NumSentFramesSinceLastUpdate / 1.0;
  this->m_Controls->fpsInLabel->setText(QString::number(fpsIn));
  this->m_Controls->fpsOutLabel->setText(QString::number(fpsOut));
  m_NumReceivedFramesSinceLastUpdate = 0;
  m_NumSentFramesSinceLastUpdate = 0;
}

void QmitkIGTLDeviceSetupConnectionWidget::OnLogMessageDetailsCheckBoxClicked()
{
  if( this->m_IGTLDevice.IsNull() )
  {
    MITK_WARN << "Logging information not passed down to Message Provider.";
    return;
  }
  else
  {
    this->m_IGTLDevice->SetLogMessages( this->m_Controls->logMessageDetailsCheckBox->isChecked() );
  }
}
