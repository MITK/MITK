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

//poco headers
#include <Poco/Path.h>

const std::string QmitkIGTLDeviceSourceManagementWidget::VIEW_ID =
    "org.mitk.views.igtldevicesourcemanagementwidget";

QmitkIGTLDeviceSourceManagementWidget::QmitkIGTLDeviceSourceManagementWidget(
    QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f), m_OutputChanged(false)
{
  m_Controls = NULL;
  m_OutputMutex = itk::FastMutexLock::New();
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

  //every 100ms the logging window has to be updated
  m_UpdateLoggingWindowTimer.setInterval(100);

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
    connect( m_Controls->butConnectWithServer, SIGNAL(clicked()),
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
    connect( &m_UpdateLoggingWindowTimer, SIGNAL(timeout()),
             this, SLOT(OnUpdateLoggingWindow()));
  }

//  //main widget page:
//  connect( (QObject*)(m_Controls->m_AddTool), SIGNAL(clicked()), this, SLOT(OnAddTool()) );
//  connect( (QObject*)(m_Controls->m_DeleteTool), SIGNAL(clicked()), this, SLOT(OnDeleteTool()) );
//  connect( (QObject*)(m_Controls->m_EditTool), SIGNAL(clicked()), this, SLOT(OnEditTool()) );
//  connect( (QObject*)(m_Controls->m_LoadStorage), SIGNAL(clicked()), this, SLOT(OnLoadStorage()) );
//  connect( (QObject*)(m_Controls->m_SaveStorage), SIGNAL(clicked()), this, SLOT(OnSaveStorage()) );
//  connect( (QObject*)(m_Controls->m_LoadTool), SIGNAL(clicked()), this, SLOT(OnLoadTool()) );
//  connect( (QObject*)(m_Controls->m_SaveTool), SIGNAL(clicked()), this, SLOT(OnSaveTool()) );
//  connect( (QObject*)(m_Controls->m_CreateNewStorage), SIGNAL(clicked()), this, SLOT(OnCreateStorage()) );


//  //widget page "add tool":
//  connect( (QObject*)(m_Controls->m_ToolCreationWidget), SIGNAL(Canceled()), this, SLOT(OnAddToolCancel()) );
//  connect( (QObject*)(m_Controls->m_ToolCreationWidget), SIGNAL(NavigationToolFinished()), this, SLOT(OnAddToolSave()) );
}

//void QmitkIGTLDeviceSourceManagementWidget::OnLoadTool()
//{
//  if(m_IGTLDeviceSource->isLocked())
//  {
//    MessageBox("Storage is locked, cannot modify it. Maybe the tracking device which uses this storage is connected. If you want to modify the storage please disconnect the device first.");
//   return;
//  }
//  mitk::NavigationToolReader::Pointer myReader = mitk::NavigationToolReader::New();
//  std::string filename = QFileDialog::getOpenFileName(NULL,tr("Add Navigation Tool"), "/", "*.IGTTool").toAscii().data();
//  if (filename == "") return;
//  mitk::NavigationTool::Pointer readTool = myReader->DoRead(filename);
//  if (readTool.IsNull()) MessageBox("Error: " + myReader->GetErrorMessage());
//  else
//    {
//    if (!m_IGTLDeviceSource->AddTool(readTool))
//      {
//      MessageBox("Error: Can't add tool!");
//      m_DataStorage->Remove(readTool->GetDataNode());
//      }
//    UpdateToolTable();
//    }
//}

//void QmitkIGTLDeviceSourceManagementWidget::OnSaveTool()
//{
//    //if no item is selected, show error message:
//    if (m_Controls->m_ToolList->currentItem() == NULL) {MessageBox("Error: Please select tool first!");return;}

//    mitk::NavigationToolWriter::Pointer myWriter = mitk::NavigationToolWriter::New();
//    std::string filename = QFileDialog::getSaveFileName(NULL,tr("Save Navigation Tool"), "/", "*.IGTTool").toAscii().data();
//    if (filename == "") return;
//    if (!myWriter->DoWrite(filename,m_IGTLDeviceSource->GetTool(m_Controls->m_ToolList->currentIndex().row())))
//      MessageBox("Error: "+ myWriter->GetErrorMessage());

//}



void QmitkIGTLDeviceSourceManagementWidget::Initialize(
    mitk::DataStorage* /*dataStorage*/)
{
//  m_DataStorage = dataStorage;
//  m_Controls->m_ToolCreationWidget->Initialize(m_DataStorage,"Tool0");
}

void QmitkIGTLDeviceSourceManagementWidget::LoadSource(
    mitk::IGTLDeviceSource::Pointer sourceToLoad)
{
  if(sourceToLoad.IsNotNull())
  {
    this->m_IGTLDeviceSource = sourceToLoad;
    this->m_IGTLClient = (mitk::IGTLClient*)this->m_IGTLDeviceSource->GetIGTLDevice();
    m_Controls->selectedSourceLabel->setText(m_IGTLDeviceSource->GetName().c_str());

    //add observer for new message receiving
//    typedef itk::MemberCommand< QmitkIGTLDeviceSourceManagementWidget > CurCommandType;
//    CurCommandType::Pointer messageReceivedCommand = CurCommandType::New();
//    messageReceivedCommand->SetCallbackFunction(
//      this, &QmitkIGTLDeviceSourceManagementWidget::OnMessageReceived );
//    this->m_IGTLClient->AddObserver(mitk::MessageReceivedEvent(), messageReceivedCommand);

    typedef itk::MemberCommand< QmitkIGTLDeviceSourceManagementWidget > CurCommandType;
    m_MessageReceivedCommand = CurCommandType::New();
    m_MessageReceivedCommand->SetCallbackFunction(
      this, &QmitkIGTLDeviceSourceManagementWidget::OnMessageReceived );
    this->m_IGTLClient->AddObserver(mitk::MessageReceivedEvent(), m_MessageReceivedCommand);

    //Fill the commands combo box with all available commands
    FillCommandsComboBox();

    //enable the controls of this widget
    EnableSourceControls();

    //start to update the logging window
    this->m_UpdateLoggingWindowTimer.start();
  }
  else
  {
    m_IGTLDeviceSource = NULL;
    DisableSourceControls();
  }
  //reset the loggin text edit
  ResetOutput();
}

////##################################################################################
////############################## slots: main widget ################################
////##################################################################################

//void QmitkIGTLDeviceSourceManagementWidget::OnAddTool()
//  {
//    if(m_IGTLDeviceSource->isLocked())
//    {
//      MessageBox("Storage is locked, cannot modify it. Maybe the tracking device which uses this storage is connected. If you want to modify the storage please disconnect the device first.");
//      return;
//    }
//    QString defaultIdentifier = "NavigationTool#"+QString::number(m_IGTLDeviceSource->GetToolCount());
//    m_Controls->m_ToolCreationWidget->Initialize(m_DataStorage,defaultIdentifier.toStdString());
//    m_edit = false;
//    m_Controls->m_MainWidgets->setCurrentIndex(1);
//  }

//void QmitkIGTLDeviceSourceManagementWidget::OnDeleteTool()
//  {
//    //first: some checks
//    if(m_IGTLDeviceSource->isLocked())
//    {
//      MessageBox("Storage is locked, cannot modify it. Maybe the tracking device which uses this storage is connected. If you want to modify the storage please disconnect the device first.");
//      return;
//    }
//    else if (m_Controls->m_ToolList->currentItem() == NULL) //if no item is selected, show error message:
//    {
//      MessageBox("Error: Please select tool first!");
//      return;
//    }

//    m_DataStorage->Remove(m_IGTLDeviceSource->GetTool(m_Controls->m_ToolList->currentIndex().row())->GetDataNode());
//    m_IGTLDeviceSource->DeleteTool(m_Controls->m_ToolList->currentIndex().row());
//    UpdateToolTable();

//  }

//void QmitkIGTLDeviceSourceManagementWidget::OnEditTool()
//  {
//    if(m_IGTLDeviceSource->isLocked())
//    {
//      MessageBox("Storage is locked, cannot modify it. Maybe the tracking device which uses this storage is connected. If you want to modify the storage please disconnect the device first.");
//      return;
//    }
//    else if (m_Controls->m_ToolList->currentItem() == NULL) //if no item is selected, show error message:
//    {
//      MessageBox("Error: Please select tool first!");
//      return;
//    }
//    mitk::NavigationTool::Pointer selectedTool = m_IGTLDeviceSource->GetTool(m_Controls->m_ToolList->currentIndex().row());
//    m_Controls->m_ToolCreationWidget->SetDefaultData(selectedTool);
//    m_edit = true;
//    m_Controls->m_MainWidgets->setCurrentIndex(1);
//  }

//void QmitkIGTLDeviceSourceManagementWidget::OnCreateStorage()
//  {
//    QString storageName = QInputDialog::getText(NULL,"Storage Name","Name of the new tool storage:");
//    if (storageName.isNull()) return;
//    m_IGTLDeviceSource = mitk::NavigationToolStorage::New(this->m_DataStorage);
//    m_IGTLDeviceSource->SetName(storageName.toStdString());
//    m_Controls->m_StorageName->setText(m_IGTLDeviceSource->GetName().c_str());
//    EnableStorageControls();
//    emit NewStorageAdded(m_IGTLDeviceSource, storageName.toStdString());
//  }

//void QmitkIGTLDeviceSourceManagementWidget::OnLoadStorage()
//  {
//    mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(m_DataStorage);
//    std::string filename = QFileDialog::getOpenFileName(NULL, tr("Open Navigation Tool Storage"), "/", tr("IGT Tool Storage (*.IGTToolStorage)")).toStdString();
//    if (filename == "") return;

//    try
//    {
//      mitk::NavigationToolStorage::Pointer tempStorage = myDeserializer->Deserialize(filename);

//      if (tempStorage.IsNull()) MessageBox("Error" + myDeserializer->GetErrorMessage());
//      else
//      {
//        Poco::Path myPath = Poco::Path(filename.c_str());
//        tempStorage->SetName(myPath.getFileName()); //set the filename as name for the storage, so the user can identify it
//        this->LoadStorage(tempStorage);
//        emit NewStorageAdded(m_IGTLDeviceSource,myPath.getFileName());
//      }
//    }
//    catch (const mitk::Exception& exception)
//    {
//      MessageBox(exception.GetDescription());
//    }
//  }

//void QmitkIGTLDeviceSourceManagementWidget::OnSaveStorage()
//  {
//    //read in filename
//    QString filename = QFileDialog::getSaveFileName(NULL, tr("Save Navigation Tool Storage"), "/", tr("IGT Tool Storage (*.IGTToolStorage)"));
//    if (filename.isEmpty()) return; //canceled by the user

//    // add file extension if it wasn't added by the file dialog
//    if ( filename.right(15) != ".IGTToolStorage" ) { filename += ".IGTToolStorage"; }

//    //serialize tool storage
//    mitk::NavigationToolStorageSerializer::Pointer mySerializer = mitk::NavigationToolStorageSerializer::New();
//    if (!mySerializer->Serialize(filename.toStdString(),m_IGTLDeviceSource))
//      {
//      MessageBox("Error: " + mySerializer->GetErrorMessage());
//      return;
//      }
//    Poco::Path myPath = Poco::Path(filename.toStdString());
//    m_Controls->m_StorageName->setText(QString::fromStdString(myPath.getFileName()));

//  }


////##################################################################################
////############################## slots: add tool widget ############################
////##################################################################################

//void QmitkIGTLDeviceSourceManagementWidget::OnAddToolSave()
//  {
//    mitk::NavigationTool::Pointer newTool = m_Controls->m_ToolCreationWidget->GetCreatedTool();

//    if (m_edit) //here we edit a existing tool
//      {
//      mitk::NavigationTool::Pointer editedTool = m_IGTLDeviceSource->GetTool(m_Controls->m_ToolList->currentIndex().row());
//      editedTool->Graft(newTool);
//      }
//    else //here we create a new tool
//      {
//      m_IGTLDeviceSource->AddTool(newTool);
//      }

//    UpdateToolTable();

//    m_Controls->m_MainWidgets->setCurrentIndex(0);
//  }

//void QmitkIGTLDeviceSourceManagementWidget::OnAddToolCancel()
//  {
//    m_Controls->m_MainWidgets->setCurrentIndex(0);
//  }




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
  m_Controls->editIP->setEnabled(true);
  m_Controls->editPort->setEnabled(true);
  m_Controls->butConnectWithServer->setEnabled(true);
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
  if(m_Controls->butConnectWithServer->text() == "Connect")
  {
    QString port = m_Controls->editPort->text();
    m_IGTLClient->SetPortNumber(port.toInt());
    std::string hostname = m_Controls->editIP->text().toStdString();
    m_IGTLClient->SetHostname(hostname);
    if ( m_IGTLClient->OpenConnection() )
    {
      if ( m_IGTLClient->StartCommunication() )
      {
        m_Controls->editIP->setEnabled(false);
        m_Controls->editPort->setEnabled(false);
        m_Controls->editSend->setEnabled(true);
        m_Controls->butSend->setEnabled(true);
        m_Controls->commandsComboBox->setEnabled(true);
        m_Controls->butConnectWithServer->setText("Disconnect");
        std::stringstream s;
        s << "<br>Successfully connected to " << hostname
          << " on port " << port.toStdString();
        this->AddOutput(s.str());
      }
      else
      {
        MITK_ERROR("QmitkIGTLDeviceSourceManagementWidget") <<
                             "Could not start a communication with the"
                             "server because the client is in the wrong state";
        this->AddOutput("<br>Connection is not working.");
      }
    }
    else
    {
      MITK_ERROR("QmitkIGTLDeviceSourceManagementWidget") <<
                            "Could not connect to the server. "
                             "Please check the hostname and port.";
      this->AddOutput("<br>Connection is not working. Please Check Host and Port.");
    }
  }
  else
  {
    m_Controls->editIP->setEnabled(true);
    m_Controls->editPort->setEnabled(true);
    m_Controls->editSend->setEnabled(false);
    m_Controls->butSend->setEnabled(false);
    m_Controls->commandsComboBox->setEnabled(false);
    m_Controls->butConnectWithServer->setText("Connect");
    m_IGTLClient->CloseConnection();
    this->AddOutput("<br>Closed connection.");
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
  m_IGTLClient->SendMessage(m_CurrentCommand.GetPointer());
  std::stringstream s;
  s << "<br>Sent command with DeviceType: " << m_CurrentCommand->GetDeviceType();
  this->AddOutput(s.str());
}

void QmitkIGTLDeviceSourceManagementWidget::OnCommandChanged(
    const QString & curCommand)
{
  mitk::IGTLMessageFactory::Pointer msgFactory =
      this->m_IGTLClient->GetMessageFactory();
  //create a new message that fits to the selected get message type command
  this->m_CurrentCommand = msgFactory->GetMessageTypeNewPointer(
        curCommand.toStdString())();
  //enable the send command button
  this->m_Controls->butSendCommand->setEnabled(true);
}


void QmitkIGTLDeviceSourceManagementWidget::OnSendMessage()
{
  std::string toBeSend = m_Controls->editSend->text().toStdString();

  igtl::StringMessage::Pointer msg = igtl::StringMessage::New().GetPointer();
  msg->SetString(toBeSend);
  m_IGTLClient->SendMessage(msg.GetPointer());
  std::stringstream s;
  s << "<br>Sent message with DeviceType: " << msg->GetDeviceType();
  this->AddOutput(s.str());
//  if ( m_IGTLClient->SendMessage(msg.GetPointer()) )
//  {
//    MITK_INFO("OpenIGTLinkExample") << "Successfully sent the message.";
//  }
//  else
//  {
//    MITK_ERROR("OpenIGTLinkExample") << "Could not send the message.";
//  }
}

void QmitkIGTLDeviceSourceManagementWidget::OnMessageReceived(itk::Object* caller, const itk::EventObject&/*event*/)
{
  //get the IGTL device that invoked this event
  mitk::IGTLDevice* dev = (mitk::IGTLDevice*)caller;

  std::stringstream s;
  s << "<br>Received a message:<br>"
    << dev->GetOldestMessageInformation();

  this->AddOutput(s.str());
}

void QmitkIGTLDeviceSourceManagementWidget::OnUpdateLoggingWindow()
{
  m_OutputMutex->Lock();
//  m_Controls->m_Logging->setHtml(QString(m_Output.str().c_str()));
//  m_Controls->m_Logging->update();
  if ( m_OutputChanged )
  {
    m_Controls->m_Logging->setText(QString(m_Output.str().c_str()));
    QScrollBar* sb = m_Controls->m_Logging->verticalScrollBar();
    sb->setValue(sb->maximum());
    m_OutputChanged = false;
  }
  m_OutputMutex->Unlock();
}

void QmitkIGTLDeviceSourceManagementWidget::FillCommandsComboBox()
{
  //load the msg factory from the client (maybe this will be moved later on)
  mitk::IGTLMessageFactory::Pointer msgFactory =
      this->m_IGTLClient->GetMessageFactory();
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

void QmitkIGTLDeviceSourceManagementWidget::ResetOutput()
{
  m_OutputMutex->Lock();
  m_Output.str("");
  m_Output <<"<body style=\" font-family:\'MS Shell Dlg 2\'; font-size:7pt; font-weight:400; font-style:normal;\" bgcolor=black><span style=\"color:#ffffff;\"><u>output:</u>";
  m_Controls->m_Logging->setHtml(QString(m_Output.str().c_str()));
  m_OutputMutex->Unlock();
}

void QmitkIGTLDeviceSourceManagementWidget::AddOutput(std::string s)
{
  //print output
  m_OutputMutex->Lock();
  m_Output << s;
  m_OutputChanged = true;
  m_OutputMutex->Unlock();
}
