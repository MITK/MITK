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
  : QWidget(parent, f), m_IsClient(false)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
}


QmitkIGTLDeviceSourceManagementWidget::~QmitkIGTLDeviceSourceManagementWidget()
{
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

void QmitkIGTLDeviceSourceManagementWidget::CreateConnections()
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
    connect( m_Controls->butSend, SIGNAL(clicked()),
             this, SLOT(OnSendMessage()));
    connect( m_Controls->butSendCommand, SIGNAL(clicked()),
             this, SLOT(OnSendCommand()));
    connect( m_Controls->commandsComboBox,
             SIGNAL(currentIndexChanged(const QString &)),
             this, SLOT(OnCommandChanged(const QString &)));
  }
}

void QmitkIGTLDeviceSourceManagementWidget::LoadSource(
    mitk::IGTLDeviceSource::Pointer sourceToLoad)
{

  DisableSourceControls();

  if(sourceToLoad.IsNotNull())
  {
    this->m_IGTLDeviceSource = sourceToLoad;
    //check if the device is a server or a client
    if ( dynamic_cast<mitk::IGTLClient*>(
           this->m_IGTLDeviceSource->GetIGTLDevice()) == NULL )
    {
      m_IsClient = false;
      m_Controls->butConnect->setText("Go Online");
    }
    else
    {
      m_IsClient = true;
      m_Controls->butConnect->setText("Connect");
    }
    //get the device
    this->m_IGTLDevice = this->m_IGTLDeviceSource->GetIGTLDevice();
    m_Controls->selectedSourceLabel->setText(m_IGTLDeviceSource->GetName().c_str());

    //add observer for new message receiving
//    typedef itk::MemberCommand< QmitkIGTLDeviceSourceManagementWidget > CurCommandType;
//    CurCommandType::Pointer messageReceivedCommand = CurCommandType::New();
//    messageReceivedCommand->SetCallbackFunction(
//      this, &QmitkIGTLDeviceSourceManagementWidget::OnMessageReceived );
//    this->m_IGTLDevice->AddObserver(mitk::MessageReceivedEvent(), messageReceivedCommand);

    typedef itk::MemberCommand< QmitkIGTLDeviceSourceManagementWidget > CurCommandType;
    m_MessageReceivedCommand = CurCommandType::New();
    m_MessageReceivedCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSourceManagementWidget::OnMessageReceived );
    this->m_IGTLDevice->AddObserver(mitk::MessageReceivedEvent(), m_MessageReceivedCommand);

    typedef itk::MemberCommand< QmitkIGTLDeviceSourceManagementWidget > CurCommandType;
    CurCommandType::Pointer commandReceivedCommand = CurCommandType::New();
    commandReceivedCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSourceManagementWidget::OnCommandReceived );
    this->m_IGTLDevice->AddObserver(mitk::CommandReceivedEvent(), commandReceivedCommand);

    //Fill the commands combo box with all available commands
    FillCommandsComboBox();

    //enable the controls of this widget
    EnableSourceControls();
  }
  else
  {
    m_IGTLDeviceSource = NULL;
  }
}

void QmitkIGTLDeviceSourceManagementWidget::MessageBox(std::string s)
{
  QMessageBox msgBox;
  msgBox.setText(s.c_str());
  msgBox.exec();
}

void QmitkIGTLDeviceSourceManagementWidget::DisableSourceControls()
{
  m_Controls->selectedSourceLabel->setText("<none>");
  m_Controls->editIP->setEnabled(false);
  m_Controls->editPort->setEnabled(false);
  m_Controls->editSend->setEnabled(false);
  m_Controls->butSendCommand->setEnabled(false);
  m_Controls->fpsSpinBox->setEnabled(false);
//  m_Controls->m_AddTool->setEnabled(false);
//  m_Controls->m_LoadTool->setEnabled(false);
//  m_Controls->m_selectedLabel->setEnabled(false);
//  m_Controls->m_DeleteTool->setEnabled(false);
//  m_Controls->m_EditTool->setEnabled(false);
//  m_Controls->m_SaveTool->setEnabled(false);
//  m_Controls->m_ToolList->setEnabled(false);
//  m_Controls->m_SaveStorage->setEnabled(false);
//  m_Controls->m_ToolLabel->setEnabled(false);
}

void QmitkIGTLDeviceSourceManagementWidget::EnableSourceControls()
{
  if ( this->m_IsClient )
  {
    m_Controls->editIP->setEnabled(true);
  }

  m_Controls->editPort->setEnabled(true);
  m_Controls->butConnect->setEnabled(true);
  m_Controls->fpsSpinBox->setEnabled(true);
//  m_Controls->editSend->setEnabled(false);
//  m_Controls->m_AddTool->setEnabled(true);
//  m_Controls->m_LoadTool->setEnabled(true);
//  m_Controls->m_selectedLabel->setEnabled(true);
//  m_Controls->m_DeleteTool->setEnabled(true);
//  m_Controls->m_EditTool->setEnabled(true);
//  m_Controls->m_SaveTool->setEnabled(true);
//  m_Controls->m_ToolList->setEnabled(true);
//  m_Controls->m_SaveStorage->setEnabled(true);
//  m_Controls->m_ToolLabel->setEnabled(true);
}


void QmitkIGTLDeviceSourceManagementWidget::OnConnect()
{
  if(m_Controls->butConnect->text() == "Connect" ||
     m_Controls->butConnect->text() == "Go Online" )
  {
    QString port = m_Controls->editPort->text();
    m_IGTLDevice->SetPortNumber(port.toInt());
    std::string hostname = m_Controls->editIP->text().toStdString();
    m_IGTLDevice->SetHostname(hostname);
    if ( m_IGTLDevice->OpenConnection() )
    {
      if ( m_IGTLDevice->StartCommunication() )
      {
        this->m_Controls->editIP->setEnabled(false);
        this->m_Controls->editPort->setEnabled(false);
        this->m_Controls->editSend->setEnabled(true);
        this->m_Controls->butSend->setEnabled(true);
        this->m_Controls->commandsComboBox->setEnabled(true);
        this->m_Controls->butSendCommand->setEnabled(true);
        this->m_Controls->butConnect->setText("Disconnect");
        if ( this->m_IsClient )
        {
          MITK_INFO("IGTLDeviceSourceManagementWidget")
              << "Successfully connected to " << hostname
              << " on port " << port.toStdString();
        }
      }
      else
      {
        MITK_ERROR("QmitkIGTLDeviceSourceManagementWidget") <<
                             "Could not start a communication with the"
                             "server because the client is in the wrong state";
      }
    }
    else
    {
      MITK_ERROR("QmitkIGTLDeviceSourceManagementWidget") <<
                            "Could not connect to the server. "
                             "Please check the hostname and port.";
    }
  }
  else
  {
    if ( this->m_IsClient )
      m_Controls->editIP->setEnabled(true);
    m_Controls->editPort->setEnabled(true);
    m_Controls->editSend->setEnabled(false);
    m_Controls->butSend->setEnabled(false);
    m_Controls->butSendCommand->setEnabled(false);
    m_Controls->commandsComboBox->setEnabled(false);
    m_Controls->butConnect->setText("Connect");
    m_IGTLDevice->CloseConnection();
    MITK_INFO("QmitkIGTLDeviceSourceManagementWidget") << "Closed connection";
  }
}


void QmitkIGTLDeviceSourceManagementWidget::OnPortChanged()
{

}


void QmitkIGTLDeviceSourceManagementWidget::OnHostnameChanged()
{

}

void QmitkIGTLDeviceSourceManagementWidget::OnSendCommand()
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

  m_IGTLDevice->SendMessage(m_CurrentCommand.GetPointer());
  if ( this->m_Controls->logSendReceiveMsg->isChecked() )
  {
    MITK_INFO("IGTLDeviceSourceManagementWidget")
        << "Sent command with DeviceType: " << m_CurrentCommand->GetDeviceType();
  }
}

void QmitkIGTLDeviceSourceManagementWidget::OnCommandChanged(
    const QString & curCommand)
{
  mitk::IGTLMessageFactory::Pointer msgFactory =
      this->m_IGTLDevice->GetMessageFactory();
  //create a new message that fits to the selected get message type command
  this->m_CurrentCommand = msgFactory->CreateInstance( curCommand.toStdString());
  //enable/disable the FPS spinbox
  this->m_Controls->fpsSpinBox->setEnabled(curCommand.contains("STT_"));
}


void QmitkIGTLDeviceSourceManagementWidget::OnSendMessage()
{
  std::string toBeSend = m_Controls->editSend->text().toStdString();

  igtl::StringMessage::Pointer msg = igtl::StringMessage::New();
  msg->SetString(toBeSend);
  m_IGTLDevice->SendMessage(msg.GetPointer());
  if ( this->m_Controls->logSendReceiveMsg->isChecked() )
  {
    MITK_INFO("IGTLDeviceSourceManagementWidget")
        << "Sent message with DeviceType: " << msg->GetDeviceType();
  }
}

void QmitkIGTLDeviceSourceManagementWidget::OnMessageReceived(
    itk::Object* caller, const itk::EventObject&/*event*/)
{
  //get the IGTL device that invoked this event
  mitk::IGTLDevice* dev = (mitk::IGTLDevice*)caller;

  if ( this->m_Controls->logSendReceiveMsg->isChecked() )
  {
    MITK_INFO("IGTLDeviceSourceManagementWidget") << "Received a message: "
        << dev->GetReceiveQueue()->GetLatestMsgInformationString();
  }
}

void QmitkIGTLDeviceSourceManagementWidget::OnCommandReceived(
    itk::Object* caller, const itk::EventObject&/*event*/)
{
  //get the IGTL device that invoked this event
  mitk::IGTLDevice* dev = (mitk::IGTLDevice*)caller;

  if ( this->m_Controls->logSendReceiveMsg->isChecked() )
  {
    MITK_INFO("IGTLDeviceSourceManagementWidget") << "Received a command: "
        << dev->GetCommandQueue()->GetLatestMsgInformationString();
  }
}

void QmitkIGTLDeviceSourceManagementWidget::FillCommandsComboBox()
{
  //load the msg factory from the client (maybe this will be moved later on)
  mitk::IGTLMessageFactory::Pointer msgFactory =
      this->m_IGTLDevice->GetMessageFactory();
  //get the available commands as std::list<std::string>
  std::list<std::string> commandsList_ =
      msgFactory->GetAvailableMessageRequestTypes();
  //create a string list to convert the std::list
  QStringList commandsList;
  while ( commandsList_.size() )
  {
    commandsList.append(QString::fromStdString(commandsList_.front()));
    commandsList_.pop_front();
  }
  //fill the combo box with life
  this->m_Controls->commandsComboBox->addItems(commandsList);
}
