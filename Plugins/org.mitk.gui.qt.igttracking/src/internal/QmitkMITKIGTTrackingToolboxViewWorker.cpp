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

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkMITKIGTTrackingToolboxViewWorker.h"
#include "QmitkTrackingDeviceConfigurationWidget.h"
#include "QmitkStdMultiWidget.h"

// Qt
#include <QMessageBox>
#include <QSettings>
#include <qfiledialog.h>

// MITK
#include <mitkNavigationToolStorageDeserializer.h>
#include <mitkTrackingDeviceSourceConfigurator.h>
#include <mitkTrackingVolumeGenerator.h>
#include <mitkNDITrackingDevice.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNavigationToolStorageSerializer.h>
//#include <mitkProgressBar.h>
#include <mitkIOUtil.h>
#include <mitkLog.h>
#include <usModule.h>
#include <mitkTrackingDeviceTypeCollection.h>
#include <mitkUnspecifiedTrackingTypeInformation.h>
#include "mitkNDIAuroraTypeInformation.h"

// vtk
#include <vtkSphereSource.h>

//for exceptions
#include <mitkIGTException.h>
#include <mitkIGTIOException.h>

//for Microservice
#include "mitkPluginActivator.h"
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include "usServiceReference.h"

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
  //mitk::ProgressBar::GetInstance()->AddStepsToDo(2);
  mitk::NavigationToolStorage::Pointer autoDetectedStorage = mitk::NavigationToolStorage::New(m_DataStorage);
  try
  {
    mitk::NavigationToolStorage::Pointer tempStorage = m_TrackingDevice->AutoDetectTools();
    //mitk::ProgressBar::GetInstance()->Progress();
    for (int i = 0; i < tempStorage->GetToolCount(); i++) { autoDetectedStorage->AddTool(tempStorage->GetTool(i)); }
  }
  catch (mitk::Exception& e)
  {
    MITK_WARN << e.GetDescription();
    //mitk::ProgressBar::GetInstance()->Reset();
    emit AutoDetectToolsFinished(false, e.GetDescription());
    return;
  }
  m_NavigationToolStorage = autoDetectedStorage;
  //::ProgressBar::GetInstance()->Progress();
    emit AutoDetectToolsFinished(true, "");
}

void QmitkMITKIGTTrackingToolboxViewWorker::ConnectDevice()
{
  std::string message = "";
  //mitk::ProgressBar::GetInstance()->AddStepsToDo(4);

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
  //mitk::ProgressBar::GetInstance()->Progress();

  //Create Navigation Data Source with the factory class
  mitk::TrackingDeviceSourceConfigurator::Pointer myTrackingDeviceSourceFactory = mitk::TrackingDeviceSourceConfigurator::New(m_NavigationToolStorage, trackingDevice);
  m_TrackingDeviceSource = myTrackingDeviceSourceFactory->CreateTrackingDeviceSource(m_ToolVisualizationFilter);
  //mitk::ProgressBar::GetInstance()->Progress();

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

  MITK_INFO << "Number of tools: " << m_TrackingDeviceSource->GetNumberOfOutputs();
  //mitk::ProgressBar::GetInstance()->Progress();

  //The tools are maybe reordered after initialization, e.g. in case of auto-detected tools of NDI Aurora
  mitk::NavigationToolStorage::Pointer toolsInNewOrder = myTrackingDeviceSourceFactory->GetUpdatedNavigationToolStorage();

  if ((toolsInNewOrder.IsNotNull()) && (toolsInNewOrder->GetToolCount() > 0))
  {
    //so delete the old tools in wrong order and add them in the right order
    //we cannot simply replace the tool storage because the new storage is
    //not correctly initialized with the right data storage

    /*
    m_NavigationToolStorage->DeleteAllTools();
    for (int i=0; i < toolsInNewOrder->GetToolCount(); i++) {m_NavigationToolStorage->AddTool(toolsInNewOrder->GetTool(i));}

    This was replaced and thereby fixed Bug 18318 DeleteAllTools() is not Threadsafe!
    */
    for (int i = 0; i < toolsInNewOrder->GetToolCount(); i++)
    {
      m_NavigationToolStorage->AssignToolNumber(toolsInNewOrder->GetTool(i)->GetIdentifier(), i);
    }
  }

  //mitk::ProgressBar::GetInstance()->Progress();

  //connect to device
  try
  {
    m_TrackingDeviceSource->Connect();
    //mitk::ProgressBar::GetInstance()->Reset();
    //Microservice registration:
    m_TrackingDeviceSource->RegisterAsMicroservice();
    m_NavigationToolStorage->UnRegisterMicroservice();
    m_NavigationToolStorage->RegisterAsMicroservice(m_TrackingDeviceSource->GetMicroserviceID());
    m_NavigationToolStorage->LockStorage();
  }
  catch (...) //todo: change to mitk::IGTException
  {
    message = "Error on connecting the tracking device.";
    emit ConnectDeviceFinished(false, QString(message.c_str()));
    return;
  }
  emit ConnectDeviceFinished(true, QString(message.c_str()));
  //mitk::ProgressBar::GetInstance()->Reset();
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
  for (int i = 0; i < this->m_NavigationToolStorage->GetToolCount(); i++)
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
  for (int i = 0; i < this->m_NavigationToolStorage->GetToolCount(); i++)
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
  }
  catch (mitk::Exception& e)
  {
    emit DisconnectDeviceFinished(false, e.GetDescription());
  }
  emit DisconnectDeviceFinished(true, "");
}
