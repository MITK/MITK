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

#include "mitkPolhemusTrackingDevice.h"
#include "mitkPolhemusTool.h"
#include "mitkIGTConfig.h"
#include "mitkIGTTimeStamp.h"
#include "mitkIGTHardwareException.h"
#include <itksys/SystemTools.hxx>
#include <iostream>
#include <itkMutexLockHolder.h>
#include "mitkPolhemusTrackerTypeInformation.h"
#include <vtkConeSource.h>

typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;


mitk::PolhemusTrackingDevice::PolhemusTrackingDevice(): mitk::TrackingDevice()
{
  //set the type of this tracking device
  this->m_Data = mitk::PolhemusTrackerTypeInformation::GetDeviceDataPolhemusTrackerLiberty();

  this->m_MultiThreader = itk::MultiThreader::New();
  m_ThreadID = 0;

  m_Device = mitk::PolhemusInterface::New();

}

mitk::PolhemusTrackingDevice::~PolhemusTrackingDevice()
{
}

bool mitk::PolhemusTrackingDevice::IsDeviceInstalled()
{
  return true;
}

mitk::TrackingTool* mitk::PolhemusTrackingDevice::AddTool( const char* toolName)
{
  mitk::PolhemusTool::Pointer t = mitk::PolhemusTool::New();
  t->SetToolName(toolName);
  if (this->InternalAddTool(t) == false)
    return nullptr;
  return t.GetPointer();
}

bool mitk::PolhemusTrackingDevice::InternalAddTool(PolhemusTool::Pointer tool)
{
  m_AllTools.push_back(tool);
  return true;
}

bool mitk::PolhemusTrackingDevice::StartTracking()
{
  bool success = m_Device->StartTracking();
  if (success)
  {
    mitk::IGTTimeStamp::GetInstance()->Start(this);
    this->SetState(Tracking);
    this->m_StopTrackingMutex->Lock();
    this->m_StopTracking = false;
    this->m_StopTrackingMutex->Unlock();
    m_ThreadID = m_MultiThreader->SpawnThread(this->ThreadStartTracking, this);    // start a new thread that executes the TrackTools() method
    return true;
  }
  else
  {
    this->SetState(Ready);
    mitkThrowException(mitk::IGTHardwareException) << "Error while trying to start the device!";
  }
  return success;
}

bool mitk::PolhemusTrackingDevice::StopTracking()
{
  m_Device->StopTracking();
  return Superclass::StopTracking();
}


unsigned int mitk::PolhemusTrackingDevice::GetToolCount() const
{
  return (unsigned int)this->m_AllTools.size();
}


mitk::TrackingTool* mitk::PolhemusTrackingDevice::GetTool(unsigned int toolNumber) const
{
  if ( toolNumber >= this->GetToolCount())
    return nullptr;
  else
    return this->m_AllTools[toolNumber];
}


bool mitk::PolhemusTrackingDevice::OpenConnection()
{
  //reset everything
  if (m_Device.IsNull()) {m_Device = mitk::PolhemusInterface::New();}
  m_Device->Connect();
  m_Device->SetHemisphereTrackingEnabled(m_HemisphereTrackingEnabled);
  this->SetState(Ready);
  return true;
}

bool mitk::PolhemusTrackingDevice::CloseConnection()
{
  bool returnValue = true;
  if (this->GetState() == Setup)
    return true;

  returnValue = m_Device->Disconnect();

  this->SetState(Setup);
  return returnValue;
}


mitk::PolhemusInterface* mitk::PolhemusTrackingDevice::GetDevice()
{
  return m_Device;
}


std::vector<mitk::PolhemusTool::Pointer> mitk::PolhemusTrackingDevice::GetAllTools()
{
  return this->m_AllTools;
}


void mitk::PolhemusTrackingDevice::TrackTools()
{
  try
  {
    /* lock the TrackingFinishedMutex to signal that the execution rights are now transfered to the tracking thread */
    MutexLockHolder trackingFinishedLockHolder(*m_TrackingFinishedMutex); // keep lock until end of scope

    bool localStopTracking;       // Because m_StopTracking is used by two threads, access has to be guarded by a mutex. To minimize thread locking, a local copy is used here
    this->m_StopTrackingMutex->Lock();  // update the local copy of m_StopTracking
    localStopTracking = this->m_StopTracking;
    this->m_StopTrackingMutex->Unlock();
    Sleep(100);//Wait a bit until the tracker is ready...



    while ((this->GetState() == Tracking) && (localStopTracking == false))
    {
      std::vector<mitk::PolhemusInterface::trackingData> lastData = this->GetDevice()->GetLastFrame();

      if (lastData.size() != m_AllTools.size())
      {
        MITK_WARN << "Tool count is corrupt. Aborting!";
      }
      else
      {


        std::vector<mitk::PolhemusTool::Pointer> allTools = this->GetAllTools();
        for (int i = 0; i < allTools.size(); i++)
        {
          mitk::PolhemusTool::Pointer currentTool = allTools.at(i);
          currentTool->SetDataValid(true);
          currentTool->SetPosition(lastData.at(i).pos);
          currentTool->SetOrientation(lastData.at(i).rot);
          currentTool->SetIGTTimeStamp(mitk::IGTTimeStamp::GetInstance()->GetElapsed());
        }
      }
      /* Update the local copy of m_StopTracking */
      this->m_StopTrackingMutex->Lock();
      localStopTracking = m_StopTracking;
      this->m_StopTrackingMutex->Unlock();

    }
  }
  catch(...)
  {
    this->StopTracking();
    mitkThrowException(mitk::IGTHardwareException) << "Error while trying to track tools. Thread stopped.";
  }
}

ITK_THREAD_RETURN_TYPE mitk::PolhemusTrackingDevice::ThreadStartTracking(void* pInfoStruct)
{
  /* extract this pointer from Thread Info structure */
  struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
  if (pInfo == nullptr)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  if (pInfo->UserData == nullptr)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  PolhemusTrackingDevice *trackingDevice = (PolhemusTrackingDevice*)pInfo->UserData;

  if (trackingDevice != nullptr)
    trackingDevice->TrackTools();

  return ITK_THREAD_RETURN_VALUE;
}

bool mitk::PolhemusTrackingDevice::AutoDetectToolsAvailable()
{
  return true;
}

mitk::NavigationToolStorage::Pointer mitk::PolhemusTrackingDevice::AutoDetectTools()
{
  this->OpenConnection();
  std::vector<mitk::PolhemusInterface::trackingData> singeFrameData = this->m_Device->GetSingleFrame();
  MITK_INFO << "Found " << singeFrameData.size() << " tools.";
  this->CloseConnection();
  mitk::NavigationToolStorage::Pointer returnValue = mitk::NavigationToolStorage::New();
  for each (mitk::PolhemusInterface::trackingData t in singeFrameData)
  {

    mitk::DataNode::Pointer newNode = mitk::DataNode::New();
    std::stringstream name;
    name << "Sensor-" << ((int)t.id);
    newNode->SetName(name.str());

    mitk::Surface::Pointer myCone = mitk::Surface::New();
    vtkConeSource *vtkData = vtkConeSource::New();
    vtkData->SetAngle(5.0);
    vtkData->SetResolution(50);
    vtkData->SetHeight(6.0f);
    vtkData->SetRadius(2.0f);
    vtkData->SetCenter(0.0, 0.0, 0.0);
    vtkData->Update();
    myCone->SetVtkPolyData(vtkData->GetOutput());
    vtkData->Delete();
    newNode->SetData(myCone);

    mitk::NavigationTool::Pointer newTool = mitk::NavigationTool::New();
    newTool->SetDataNode(newNode);

    std::stringstream identifier;
    identifier << "AutoDetectedTool-" << ((int)t.id);
    newTool->SetIdentifier(identifier.str());

    newTool->SetTrackingDeviceType(mitk::PolhemusTrackerTypeInformation::GetDeviceDataPolhemusTrackerLiberty().Line);
    returnValue->AddTool(newTool);
  }
  return returnValue;
}

void  mitk::PolhemusTrackingDevice::SetHemisphereTrackingEnabled(bool _HemisphereTrackingEnabled)
{
  if (m_HemisphereTrackingEnabled != _HemisphereTrackingEnabled)
  {
    m_HemisphereTrackingEnabled = _HemisphereTrackingEnabled;
    this->m_Device->SetHemisphereTrackingEnabled(_HemisphereTrackingEnabled);
  }


}
