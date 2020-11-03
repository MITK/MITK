/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkIGTConnectionWidget.h"
#include "QmitkTrackingDeviceConfigurationWidget.h"

#include "mitkClaronTrackingDevice.h"
#include "mitkNDITrackingDevice.h"
#include "mitkOptitrackTrackingDevice.h"

#include "mitkNavigationToolStorageDeserializer.h"
#include "mitkTrackingDeviceSourceConfigurator.h"
#include "QmitkIGTCommonHelper.h"

#include <QFileDialog>
#include <QMessageBox>

const std::string QmitkIGTConnectionWidget::VIEW_ID = "org.mitk.views.igtconnectionwidget";

QmitkIGTConnectionWidget::QmitkIGTConnectionWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_Controls = nullptr;
  CreateQtPartControl(this);
  CreateConnections();
  m_TrackingDevice = nullptr;
  m_TrackingDeviceSource = nullptr;
  m_NavigationToolStorage = nullptr;
  m_DataStorage = nullptr;
  m_ErrorMessage = "";
}


QmitkIGTConnectionWidget::~QmitkIGTConnectionWidget()
{
}

void QmitkIGTConnectionWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
  // create GUI widgets
  m_Controls = new Ui::QmitkIGTConnectionWidgetControls;
  m_Controls->setupUi(parent);
  }
}

void QmitkIGTConnectionWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->connectButton), SIGNAL(clicked()), this, SLOT(OnConnect()) );
  }
}

void QmitkIGTConnectionWidget::OnConnect()
{
  if (m_Controls->connectButton->isChecked()) // Load tools and connect tracking device
  {
    m_Controls->connectButton->setChecked(false);
    // create TrackingDevice
    m_TrackingDevice = m_Controls->trackingDeviceConfigurationWidget->GetTrackingDevice();
    if (m_TrackingDevice.IsNotNull())
    {
      QString fileName = QFileDialog::getOpenFileName(nullptr,tr("Open Navigation tool storage"), QmitkIGTCommonHelper::GetLastFileLoadPath(), tr("Toolfile (*.tfl)"));
      QmitkIGTCommonHelper::SetLastFileLoadPathByFileName(fileName);
      if (LoadToolfile(fileName))
      {
        // Create TrackingDeviceSource and add tools
        mitk::TrackingDeviceSourceConfigurator::Pointer myTrackingDeviceSourceFactory =
          mitk::TrackingDeviceSourceConfigurator::New(this->m_NavigationToolStorage,m_TrackingDevice);
        m_TrackingDeviceSource = myTrackingDeviceSourceFactory->CreateTrackingDeviceSource();
        m_TrackingDeviceSource->Connect();
        m_TrackingDeviceSource->StartTracking();
        // change button text
        m_Controls->connectButton->setText("Disconnect");
        m_Controls->connectButton->setChecked(true);
        // disable configuration widget
        m_Controls->trackingDeviceConfigurationWidget->setEnabled(false);
        // emit connected signal
        emit TrackingDeviceConnected();
      }
      else
      {
        QString error(m_ErrorMessage.c_str());
        QMessageBox::warning(nullptr,"Warning",error);
        // reset button to unchecked
        m_Controls->connectButton->setChecked(false);
        // remove tool nodes from DataStorage
        this->RemoveToolNodes();
        // reset NavigationToolStorage
        m_NavigationToolStorage = nullptr;
      }
    }
    else
    {
      // reset button to unchecked
      m_Controls->connectButton->setChecked(false);
      MITK_ERROR<<"Could not create TrackingDevice";
    }
  }
  else // Disconnect tracking device
  {
    // disconnect TrackingDeviceSource
    if (m_TrackingDeviceSource.IsNotNull())
    {
      m_TrackingDeviceSource->StopTracking();
      m_TrackingDeviceSource->Disconnect();
    }
    // remove tool nodes from DataStorage
    this->RemoveToolNodes();
    // reset members
    m_NavigationToolStorage = nullptr;
    m_TrackingDevice = nullptr;
    m_TrackingDeviceSource = nullptr;
    // change button text
    m_Controls->connectButton->setText("Connect");
    // enable configuration widget
    m_Controls->trackingDeviceConfigurationWidget->setEnabled(true);
    // emit disconnected signal
    emit TrackingDeviceDisconnected();
  }
}

bool QmitkIGTConnectionWidget::LoadToolfile(QString qFilename)
{
  if (m_DataStorage.IsNotNull())
  {
    std::string filename = qFilename.toStdString();
    mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(this->m_DataStorage);
    mitk::NavigationToolStorage::Pointer tempStorage = myDeserializer->Deserialize(filename);
    m_NavigationToolStorage = tempStorage;

    if (tempStorage.IsNull())
    {
      m_ErrorMessage = myDeserializer->GetErrorMessage();
      return false;
    }

    // check if there are tools in the storage
    mitk::TrackingDeviceType lastDevice;
    if (tempStorage->GetToolCount()>0)
    {
      lastDevice = tempStorage->GetTool(0)->GetTrackingDeviceType();
    }
    else
    {
      m_ErrorMessage = "Error: Didn't find a tool in the storage. Do you want to navigate without even an instrument?";
      return false;
    }
    //check if all tools are from the same device
    for (unsigned int i=1; i<tempStorage->GetToolCount(); i++)
    {
      if (lastDevice!=tempStorage->GetTool(i)->GetTrackingDeviceType())
      {
        m_ErrorMessage = "Error: Toolfile contains tools of different tracking devices which is not acceptable for this application.";
        return false;
      }
      else lastDevice = tempStorage->GetTool(i)->GetTrackingDeviceType();
    }
    // check if tracking device typ of tools corresponds with chosen tracking device
    if (m_TrackingDevice->GetType()!=tempStorage->GetTool(0)->GetTrackingDeviceType())
    {
      m_ErrorMessage = "Tools are not compliant with this tracking device. Please use correct toolfile for specified device.";
      return false;
    }
    m_NavigationToolStorage = tempStorage;
    return true;
  }
  else
  {
    m_ErrorMessage = "Error: No DataStorage available! Make sure the widget is initialized with a DataStorage";
    return false;
  }
}

void QmitkIGTConnectionWidget::RemoveToolNodes()
{
  for (unsigned int i=0; i<m_NavigationToolStorage->GetToolCount(); i++)
  {
    mitk::DataNode::Pointer currentNode = m_NavigationToolStorage->GetTool(i)->GetDataNode();
    if (currentNode.IsNotNull())
    {
      m_DataStorage->Remove(currentNode);
    }
  }
}

mitk::TrackingDeviceSource::Pointer QmitkIGTConnectionWidget::GetTrackingDeviceSource()
{
  return m_TrackingDeviceSource;
}

void QmitkIGTConnectionWidget::SetDataStorage( mitk::DataStorage::Pointer dataStorage )
{
  m_DataStorage = dataStorage;
}

mitk::NavigationToolStorage::Pointer QmitkIGTConnectionWidget::GetNavigationToolStorage()
{
  return m_NavigationToolStorage;
}
