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

#include "QmitkIGTLStreamingManagementWidget.h"

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

const std::string QmitkIGTLStreamingManagementWidget::VIEW_ID =
    "org.mitk.views.igtldevicesourcemanagementwidget";

QmitkIGTLStreamingManagementWidget::QmitkIGTLStreamingManagementWidget(
    QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f), m_IsClient(false)
{
  m_Controls = NULL;
  this->m_IGTLDevice = NULL;
  CreateQtPartControl(this);
}


QmitkIGTLStreamingManagementWidget::~QmitkIGTLStreamingManagementWidget()
{
   this->RemoveObserver();
}

void QmitkIGTLStreamingManagementWidget::RemoveObserver()
{
   if (this->m_IGTLDevice.IsNotNull())
   {
      this->m_IGTLDevice->RemoveObserver(m_LostConnectionObserverTag);
      this->m_IGTLDevice->RemoveObserver(m_NewConnectionObserverTag);
      this->m_IGTLDevice->RemoveObserver(m_StateModifiedObserverTag);
   }
}

void QmitkIGTLStreamingManagementWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkIGTLStreamingManagementWidgetControls;
    // setup GUI widgets
    m_Controls->setupUi(parent);
  }

  //connect slots with signals
  CreateConnections();
}

void QmitkIGTLStreamingManagementWidget::CreateConnections()
{
  if (m_Controls)
  {
    connect( (QObject*)(m_Controls->messageSourceSelectionWidget),
             SIGNAL(IGTLMessageSourceSelected(mitk::IGTLMessageSource::Pointer)),
             this,
             SLOT(SourceSelected(mitk::IGTLMessageSource::Pointer)) );
    connect( m_Controls->startStreamPushButton, SIGNAL(clicked()),
             this, SLOT(OnStartStreaming()));
    connect( m_Controls->stopStreamPushButton, SIGNAL(clicked()),
             this, SLOT(OnStopStreaming()));
  }
  //this is used for thread seperation, otherwise the worker thread would change the ui elements
  //which would cause an exception
  connect(this, SIGNAL(AdaptGUIToStateSignal()), this, SLOT(AdaptGUIToState()));
  connect(this, SIGNAL(SelectSourceAndAdaptGUISignal()), this, SLOT(SelectSourceAndAdaptGUI()));
}

void QmitkIGTLStreamingManagementWidget::AdaptGUIToState()
{
  if (this->m_IGTLMsgProvider.IsNotNull())
  {
    //get the state of the device
    mitk::IGTLDevice::IGTLDeviceState state =
        this->m_IGTLDevice->GetState();

    switch (state)
    {
      case mitk::IGTLDevice::Setup:
      case mitk::IGTLDevice::Ready:
        m_Controls->messageSourceSelectionWidget->setEnabled(false);
        m_Controls->selectedSourceLabel->setText("<none>");
        m_Controls->startStreamPushButton->setEnabled(false);
        m_Controls->selectedSourceLabel->setEnabled(false);
        m_Controls->label->setEnabled(false);
        m_Controls->stopStreamPushButton->setEnabled(false);
        m_Controls->fpsLabel->setEnabled(false);
        m_Controls->fpsSpinBox->setEnabled(false);
        break;
      case mitk::IGTLDevice::Running:
        //check the number of connections of the device, a server can be in
        //the running state even if there is no connected device, this part of
        //the GUI shall just be available when there is a connection
        if ( this->m_IGTLDevice->GetNumberOfConnections() == 0 )
        {
          m_Controls->messageSourceSelectionWidget->setEnabled(false);
          m_Controls->selectedSourceLabel->setText("<none>");
          m_Controls->startStreamPushButton->setEnabled(false);
          m_Controls->selectedSourceLabel->setEnabled(false);
          m_Controls->label->setEnabled(false);
          m_Controls->stopStreamPushButton->setEnabled(false);
          m_Controls->fpsLabel->setEnabled(false);
          m_Controls->fpsSpinBox->setEnabled(false);
        }
        else //there is a connection
        {
          //check if the user already selected a source to stream
          if ( this->m_IGTLMsgSource.IsNull() ) // he did not so far
          {
            m_Controls->messageSourceSelectionWidget->setEnabled(true);
            m_Controls->selectedSourceLabel->setText("<none>");
            m_Controls->startStreamPushButton->setEnabled(false);
            m_Controls->selectedSourceLabel->setEnabled(false);
            m_Controls->label->setEnabled(false);
            m_Controls->stopStreamPushButton->setEnabled(false);
            m_Controls->fpsLabel->setEnabled(false);
            m_Controls->fpsSpinBox->setEnabled(false);
          }
          else //user already selected a source
          {
            QString nameOfSource =
                QString::fromStdString(m_IGTLMsgSource->GetName());
            m_Controls->messageSourceSelectionWidget->setEnabled(true);
            m_Controls->selectedSourceLabel->setText(nameOfSource);
            m_Controls->selectedSourceLabel->setEnabled(true);
            m_Controls->label->setEnabled(true);

            //check if the streaming is already running
            if (this->m_IGTLMsgProvider->IsStreaming())
            {
              m_Controls->startStreamPushButton->setEnabled(false);
              m_Controls->stopStreamPushButton->setEnabled(true);
              m_Controls->fpsLabel->setEnabled(false);
              m_Controls->fpsSpinBox->setEnabled(false);
            }
            else
            {
              m_Controls->startStreamPushButton->setEnabled(true);
              m_Controls->stopStreamPushButton->setEnabled(false);
              m_Controls->fpsLabel->setEnabled(true);
              m_Controls->fpsSpinBox->setEnabled(true);
            }
          }
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

void QmitkIGTLStreamingManagementWidget::LoadSource(
    mitk::IGTLMessageProvider::Pointer provider)
{
  //reset the GUI
  DisableSourceControls();

  if ( provider.IsNull() )
    return;

  //reset the observers
  this->RemoveObserver();


    this->m_IGTLMsgProvider = provider;

    //get the device
    this->m_IGTLDevice = this->m_IGTLMsgProvider->GetIGTLDevice();

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

    typedef itk::SimpleMemberCommand< QmitkIGTLStreamingManagementWidget > CurCommandType;
//    CurCommandType::Pointer messageReceivedCommand = CurCommandType::New();
//    messageReceivedCommand->SetCallbackFunction(
//      this, &QmitkIGTLStreamingManagementWidget::OnMessageReceived );
//    this->m_MessageReceivedObserverTag =
//        this->m_IGTLDevice->AddObserver(mitk::MessageReceivedEvent(), messageReceivedCommand);

//    CurCommandType::Pointer commandReceivedCommand = CurCommandType::New();
//    commandReceivedCommand->SetCallbackFunction(
//      this, &QmitkIGTLStreamingManagementWidget::OnCommandReceived );
//    this->m_CommandReceivedObserverTag =
//        this->m_IGTLDevice->AddObserver(mitk::CommandReceivedEvent(), commandReceivedCommand);

    CurCommandType::Pointer connectionLostCommand = CurCommandType::New();
    connectionLostCommand->SetCallbackFunction(
      this, &QmitkIGTLStreamingManagementWidget::OnLostConnection );
    this->m_LostConnectionObserverTag = this->m_IGTLDevice->AddObserver(
          mitk::LostConnectionEvent(), connectionLostCommand);

    CurCommandType::Pointer newConnectionCommand = CurCommandType::New();
    newConnectionCommand->SetCallbackFunction(
      this, &QmitkIGTLStreamingManagementWidget::OnNewConnection );
    this->m_NewConnectionObserverTag = this->m_IGTLDevice->AddObserver(
          mitk::NewClientConnectionEvent(), newConnectionCommand);

    CurCommandType::Pointer stateModifiedCommand = CurCommandType::New();
    stateModifiedCommand->SetCallbackFunction(
      this, &QmitkIGTLStreamingManagementWidget::OnDeviceStateChanged );
    this->m_StateModifiedObserverTag = this->m_IGTLDevice->AddObserver(
          itk::ModifiedEvent(), stateModifiedCommand);

  this->AdaptGUIToState();
}

void QmitkIGTLStreamingManagementWidget::DisableSourceControls()
{
  m_Controls->selectedSourceLabel->setText("<none>");
  m_Controls->startStreamPushButton->setEnabled(false);
  m_Controls->stopStreamPushButton->setEnabled(false);
  m_Controls->fpsLabel->setEnabled(false);
  m_Controls->fpsSpinBox->setEnabled(false);
  m_Controls->selectedSourceLabel->setEnabled(false);
  m_Controls->label->setEnabled(false);
  m_Controls->messageSourceSelectionWidget->setEnabled(false);
}

void QmitkIGTLStreamingManagementWidget::SourceSelected(
    mitk::IGTLMessageSource::Pointer source)
{
  //reset everything
  this->DisableSourceControls();

  if (source.IsNotNull()) //no source selected
  {
    this->m_IGTLMsgSource = source;
  }

  this->AdaptGUIToState();
}

void QmitkIGTLStreamingManagementWidget::OnStartStreaming()
{
  unsigned int fps = this->m_Controls->fpsSpinBox->value();
  this->m_IGTLMsgProvider->StartStreamingOfSource(this->m_IGTLMsgSource, fps);
  this->AdaptGUIToState();
}

void QmitkIGTLStreamingManagementWidget::OnStopStreaming()
{
  this->m_IGTLMsgProvider->StopStreamingOfSource(this->m_IGTLMsgSource);
  this->AdaptGUIToState();
}

void QmitkIGTLStreamingManagementWidget::OnMessageReceived()
{
}

void QmitkIGTLStreamingManagementWidget::OnCommandReceived()
{
}

void QmitkIGTLStreamingManagementWidget::OnDeviceStateChanged()
{
   emit AdaptGUIToStateSignal();
}

void QmitkIGTLStreamingManagementWidget::OnLostConnection()
{
   emit AdaptGUIToStateSignal();
}

void QmitkIGTLStreamingManagementWidget::OnNewConnection()
{
   emit SelectSourceAndAdaptGUISignal();
}

void QmitkIGTLStreamingManagementWidget::SelectSourceAndAdaptGUI()
{
   //get the current selection and call SourceSelected which will call AdaptGUI
   mitk::IGTLMessageSource::Pointer curSelSrc =
      m_Controls->messageSourceSelectionWidget->GetSelectedIGTLMessageSource();
   SourceSelected(curSelSrc);
}
