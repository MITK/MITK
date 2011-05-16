/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:14:45 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkIGTConnectionWidget.h"
#include "QmitkTrackingDeviceConfigurationWidget.h"

#include "mitkClaronTrackingDevice.h"
#include "mitkNDITrackingDevice.h"

#include "mitkNavigationToolStorageDeserializer.h"
#include "mitkTrackingDeviceSourceConfigurator.h"

#include <QFileDialog>
#include <QMessageBox>

const std::string QmitkIGTConnectionWidget::VIEW_ID = "org.mitk.views.igtconnectionwidget";

QmitkIGTConnectionWidget::QmitkIGTConnectionWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
  CreateConnections();
  m_TrackingDevice = NULL;
  m_TrackingDeviceSource = NULL;
  m_NavigationToolStorage = NULL;
  m_DataStorage = NULL;
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
  // configure trackingDeviceConfigurationWidget
  m_Controls->trackingDeviceConfigurationWidget->EnableAdvancedUserControl(false);
  }
}

void QmitkIGTConnectionWidget::CreateConnections()
{
  if ( m_Controls )
  {
    //connect( (QObject*)(m_Controls->selectTrackingDeviceComboBox), SIGNAL(currentIndexChanged(int)), this, SLOT(OnTrackingDeviceChanged()) );
    connect( (QObject*)(m_Controls->connectButton), SIGNAL(clicked()), this, SLOT(OnConnect()) );

    ////set a few UI components depending on Windows / Linux
    //#ifdef WIN32
    //m_Controls->portTypeLabelPolaris->setVisible(false);
    //m_Controls->portTypePolaris->setVisible(false);
    //m_Controls->portTypeLabelAurora->setVisible(false);
    //m_Controls->portTypeAurora->setVisible(false);
    //#else
    //m_Controls->comPortLabelAurora->setText("Port Nr:");
    //m_Controls->comPortLabelPolaris->setText("Port Nr:");
    //m_Controls->comPortAurora->setPrefix("");
    //m_Controls->comPortPolaris->setPrefix("");
    //#endif
  }
}

void QmitkIGTConnectionWidget::OnTrackingDeviceChanged()
{
  ////show the correspondig widget for configuring the TrackingDevice
  //m_Controls->deviceConfigurationWidget->setCurrentIndex(m_Controls->selectTrackingDeviceComboBox->currentIndex());
}

void QmitkIGTConnectionWidget::OnConnect()
{
  if (m_Controls->connectButton->isChecked()) // Load tools and connect tracking device
  {
    m_Controls->connectButton->setChecked(false);
    QString fileName = QFileDialog::getOpenFileName(NULL,tr("Open Navigation tool storage"), "/", tr("Toolfile (*.tfl)"));
    if (LoadToolfile(fileName))
    {
      // create TrackingDevice
      m_TrackingDevice = m_Controls->trackingDeviceConfigurationWidget->GetTrackingDevice();
      if (m_TrackingDevice.IsNotNull())
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
      }
      else
      {
        // reset button to unchecked
        m_Controls->connectButton->setChecked(false);
        MITK_ERROR<<"Could not create TrackingDevice";
      }
    }
    else
    {
      QString error(m_ErrorMessage.c_str());
      QMessageBox::warning(NULL,"Warning",error);
      // reset button to unchecked
      m_Controls->connectButton->setChecked(false);
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
    // change button text
    m_Controls->connectButton->setText("Connect");
  }
}

bool QmitkIGTConnectionWidget::LoadToolfile(QString qFilename)
{
  if (m_DataStorage.IsNotNull())
  {
    std::string filename = qFilename.toStdString();
    mitk::NavigationToolStorageDeserializer::Pointer myDeserializer = mitk::NavigationToolStorageDeserializer::New(this->m_DataStorage);
    mitk::NavigationToolStorage::Pointer tempStorage = myDeserializer->Deserialize(filename);

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
    for (int i=1; i<tempStorage->GetToolCount(); i++)
    {
      if (lastDevice!=tempStorage->GetTool(i)->GetTrackingDeviceType()) 
      {
        m_ErrorMessage = "Error: Toolfile contains tools of different tracking devices which is not acceptable for this application.";
        return false;
      }
      else lastDevice = tempStorage->GetTool(i)->GetTrackingDeviceType();
    }
    //// check if tool device type and tracking device type are equal
    //if (lastDevice!=m_Controls->trackingDeviceConfigurationWidget->)
    //{
    //  m_ErrorMessage = "Error: Tools are not applicable for the chosen device";
    //  return false;
    //}

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
  for (int i=0; i<m_NavigationToolStorage->GetToolCount(); i++)
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

//mitk::TrackingDevice::Pointer QmitkIGTConnectionWidget::ConstructTrackingDevice()
//  {
//  mitk::TrackingDevice::Pointer returnValue;
//  //#### Step 1: configure tracking device:
//  if (m_Controls->selectTrackingDeviceComboBox->currentIndex()==0)//NDI Polaris
//      {
//      if(m_Controls->polarisMode5D->isChecked()) //5D Tracking
//        {
//        //not yet in the open source part so we'll only get NULL here.
//        returnValue = ConfigureNDI5DTrackingDevice();
//        }
//      else //6D Tracking
//        {
//        returnValue = ConfigureNDI6DTrackingDevice();
//        returnValue->SetType(mitk::NDIPolaris);
//        }
//      }
//  else if (m_Controls->selectTrackingDeviceComboBox->currentIndex()==1)//NDI Aurora
//        {
//        returnValue = ConfigureNDI6DTrackingDevice();
//        returnValue->SetType(mitk::NDIAurora);
//        }
//  else if (m_Controls->selectTrackingDeviceComboBox->currentIndex()==2)//ClaronTechnology MicronTracker 2
//        {
//        returnValue = mitk::ClaronTrackingDevice::New();
//        }
//  else
//  {
//    returnValue = NULL;
//  }
//  return returnValue;
//  }
//
//mitk::TrackingDevice::Pointer QmitkIGTConnectionWidget::ConfigureNDI5DTrackingDevice()
//  {
//  return NULL;
//  }
//
//mitk::TrackingDevice::Pointer QmitkIGTConnectionWidget::ConfigureNDI6DTrackingDevice()
//  {
//  mitk::NDITrackingDevice::Pointer tempTrackingDevice = mitk::NDITrackingDevice::New();
//    
//  //build prefix (depends on linux/win)
//  QString prefix = "";
//  #ifdef WIN32
//  prefix ="COM";
//  #else
//  if (m_Controls->selectTrackingDeviceComboBox->currentIndex()==1) //Aurora
//    prefix = m_Controls->portTypeAurora->currentText();  
//  else //Polaris
//    prefix = m_Controls->portTypePolaris->currentText();
//  #endif
//  //get port
//  int port = 0;
//  if (m_Controls->selectTrackingDeviceComboBox->currentIndex()==1) port = m_Controls->comPortAurora->value();
//  else port = m_Controls->comPortPolaris->value();
//  //build port name string
//  QString portName = prefix + QString::number(port);
//
//  tempTrackingDevice->SetDeviceName(portName.toStdString()); //set the port name
//  mitk::TrackingDevice::Pointer returnValue = static_cast<mitk::TrackingDevice*>(tempTrackingDevice);
//  return returnValue;
//  }

mitk::NavigationToolStorage::Pointer QmitkIGTConnectionWidget::GetNavigationToolStorage()
{
  return m_NavigationToolStorage;
}
