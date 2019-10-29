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

// Qmitk
#include "QmitkMITKIGTTrackingToolboxViewWorker.h"

#include <mitkTrackingDeviceSourceConfigurator.h>

QmitkMITKIGTTrackingToolboxViewWorker::QmitkMITKIGTTrackingToolboxViewWorker()
{
}

QmitkMITKIGTTrackingToolboxViewWorker::~QmitkMITKIGTTrackingToolboxViewWorker()
{
}

void QmitkMITKIGTTrackingToolboxViewWorker::SetWorkerMethod(WorkerMethod w)
{
  m_WorkerMethod = w;
}

void QmitkMITKIGTTrackingToolboxViewWorker::SetTrackingDevice(mitk::TrackingDevice::Pointer t)
{
  m_TrackingDevice = t;
}

void QmitkMITKIGTTrackingToolboxViewWorker::SetDataStorage(mitk::DataStorage::Pointer d)
{
  m_DataStorage = d;
}

void QmitkMITKIGTTrackingToolboxViewWorker::SetInverseMode(bool mode)
{
  m_InverseMode = mode;
}

void QmitkMITKIGTTrackingToolboxViewWorker::SetTrackingDeviceData(mitk::TrackingDeviceData d)
{
  m_TrackingDeviceData = d;
}

void QmitkMITKIGTTrackingToolboxViewWorker::SetNavigationToolStorage(mitk::NavigationToolStorage::Pointer n)
{
  m_NavigationToolStorage = n;
}

//! [Thread 7]
void QmitkMITKIGTTrackingToolboxViewWorker::ThreadFunc()
{
  switch (m_WorkerMethod)
  {
  case eAutoDetectTools:
    this->AutoDetectTools();
    break;
  case eConnectDevice:
    this->ConnectDevice();
    break;
  case eStartTracking:
    this->StartTracking();
    break;
  case eStopTracking:
    this->StopTracking();
    break;
  case eDisconnectDevice:
    this->DisconnectDevice();
    break;
  default:
    MITK_WARN << "Undefined worker method was set ... something went wrong!";
    break;
  }
}
//! [Thread 7]

void QmitkMITKIGTTrackingToolboxViewWorker::AutoDetectTools()
{
  mitk::NavigationToolStorage::Pointer autoDetectedStorage = mitk::NavigationToolStorage::New(m_DataStorage);
  try
  {
    mitk::NavigationToolStorage::Pointer tempStorage = m_TrackingDevice->AutoDetectTools();
    for (unsigned int i = 0; i < tempStorage->GetToolCount(); i++) { autoDetectedStorage->AddTool(tempStorage->GetTool(i)); }
  }
  catch (mitk::Exception& e)
  {
    MITK_WARN << e.GetDescription();
    emit AutoDetectToolsFinished(false, e.GetDescription());
    return;
  }
  m_NavigationToolStorage = nullptr;
  m_NavigationToolStorage = autoDetectedStorage;
  emit AutoDetectToolsFinished(true, "");
  MITK_INFO << "AutoDetect Tools Finished.";
}

void QmitkMITKIGTTrackingToolboxViewWorker::ConnectDevice()
{
  std::string message = "";

  //build the IGT pipeline
  mitk::TrackingDevice::Pointer trackingDevice = m_TrackingDevice;
  trackingDevice->SetData(m_TrackingDeviceData);

  //set device to rotation mode transposed becaus we are working with VNL style quaternions
  if (m_InverseMode)
  {
    trackingDevice->SetRotationMode(mitk::TrackingDevice::RotationTransposed);
  }

  //Get Tracking Volume Data
  mitk::TrackingDeviceData data = m_TrackingDeviceData;

  //Create Navigation Data Source with the factory class
  mitk::TrackingDeviceSourceConfigurator::Pointer myTrackingDeviceSourceFactory = mitk::TrackingDeviceSourceConfigurator::New(m_NavigationToolStorage, trackingDevice);

  m_TrackingDeviceSource = myTrackingDeviceSourceFactory->CreateTrackingDeviceSource(m_ToolVisualizationFilter);

  if (m_TrackingDeviceSource.IsNull())
  {
    message = std::string("Cannot connect to device: ") + myTrackingDeviceSourceFactory->GetErrorMessage();
    emit ConnectDeviceFinished(false, QString(message.c_str()));
    return;
  }

  //set filter to rotation mode transposed becaus we are working with VNL style quaternions
  if (m_InverseMode)
    m_ToolVisualizationFilter->SetRotationMode(mitk::NavigationDataObjectVisualizationFilter::RotationTransposed);

  //First check if the created object is valid
  if (m_TrackingDeviceSource.IsNull())
  {
    message = myTrackingDeviceSourceFactory->GetErrorMessage();
    emit ConnectDeviceFinished(false, QString(message.c_str()));
    return;
  }

  MITK_INFO << "Connected device with " << m_TrackingDeviceSource->GetNumberOfOutputs() << " tools.";

  //connect to device
  try
  {
    m_TrackingDeviceSource->Connect();
    //Microservice registration:
    m_TrackingDeviceSource->SetToolMetaDataCollection(m_NavigationToolStorage);
    m_TrackingDeviceSource->RegisterAsMicroservice();
    m_NavigationToolStorage->SetSourceID(m_TrackingDeviceSource->GetMicroserviceID()); //DEPRECATED / not needed anymore because NavigationDataSource now holds a member of its tool storage. Only left for backward compatibility.
    m_NavigationToolStorage->LockStorage();
  }
  catch (...) //todo: change to mitk::IGTException
  {
    message = "Error on connecting the tracking device.";
    emit ConnectDeviceFinished(false, QString(message.c_str()));
    return;
  }
  emit ConnectDeviceFinished(true, QString(message.c_str()));
}

mitk::TrackingDeviceSource::Pointer QmitkMITKIGTTrackingToolboxViewWorker::GetTrackingDeviceSource()
{
  return this->m_TrackingDeviceSource;
}

void QmitkMITKIGTTrackingToolboxViewWorker::StartTracking()
{
  QString errorMessage = "";
  try
  {
    m_TrackingDeviceSource->StartTracking();
  }
  catch (...) //todo: change to mitk::IGTException
  {
    errorMessage += "Error while starting the tracking device!";
    emit StartTrackingFinished(false, errorMessage);
    return;
  }
  //remember the original colors of the tools
  m_OriginalColors = std::map<mitk::DataNode::Pointer, mitk::Color>();
  for (unsigned int i = 0; i < this->m_NavigationToolStorage->GetToolCount(); i++)
  {
    mitk::DataNode::Pointer currentToolNode = m_NavigationToolStorage->GetTool(i)->GetDataNode();
    float c[3];
    currentToolNode->GetColor(c);
    mitk::Color color;
    color.SetRed(c[0]);
    color.SetGreen(c[1]);
    color.SetBlue(c[2]);
    m_OriginalColors[currentToolNode] = color;
  }

  emit StartTrackingFinished(true, errorMessage);
}

void QmitkMITKIGTTrackingToolboxViewWorker::StopTracking()
{
  //stop tracking
  try
  {
    m_TrackingDeviceSource->StopTracking();
  }
  catch (mitk::Exception& e)
  {
    emit StopTrackingFinished(false, e.GetDescription());
  }

  //restore the original colors of the tools
  for (unsigned int i = 0; i < this->m_NavigationToolStorage->GetToolCount(); i++)
  {
    mitk::DataNode::Pointer currentToolNode = m_NavigationToolStorage->GetTool(i)->GetDataNode();
    if (m_OriginalColors.find(currentToolNode) == m_OriginalColors.end())
    {
      MITK_WARN << "Cannot restore original color of tool " << m_NavigationToolStorage->GetTool(i)->GetToolName();
    }
    else
    {
      currentToolNode->SetColor(m_OriginalColors[currentToolNode]);
    }
  }

  // clear map m_OriginalColors
  m_OriginalColors.clear();

  //emit signal
  emit StopTrackingFinished(true, "");
}

void QmitkMITKIGTTrackingToolboxViewWorker::DisconnectDevice()
{
  try
  {
    if (m_TrackingDeviceSource->IsTracking()) { m_TrackingDeviceSource->StopTracking(); }
    m_TrackingDeviceSource->Disconnect();
    m_TrackingDeviceSource->UnRegisterMicroservice();

    m_NavigationToolStorage->UnLockStorage();

    m_TrackingDeviceSource = nullptr;
  }
  catch (mitk::Exception& e)
  {
    emit DisconnectDeviceFinished(false, e.GetDescription());
  }
  emit DisconnectDeviceFinished(true, "");
}
