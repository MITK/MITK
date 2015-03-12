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

#include "mitkOpenIGTLinkTrackingDevice.h"
#include "mitkOpenIGTLinkTrackingTool.h"
#include "mitkIGTConfig.h"
#include "mitkIGTTimeStamp.h"
#include "mitkIGTHardwareException.h"
#include "mitkTrackingTypes.h"
#include <itksys/SystemTools.hxx>
#include <iostream>
#include <itkMutexLockHolder.h>

typedef itk::MutexLockHolder<itk::FastMutexLock> MutexLockHolder;


mitk::OpenIGTLinkTrackingDevice::OpenIGTLinkTrackingDevice(): mitk::TrackingDevice()
{
  //set the type of this tracking device
  this->m_Data = mitk::DeviceDataOpenIGTLinkTrackingDeviceConnection;

  this->m_MultiThreader = itk::MultiThreader::New();
  m_ThreadID = 0;

  m_OpenIGTLinkClient = mitk::IGTLClient::New();
  m_OpenIGTLinkClient->SetName("OpenIGTLink Tracking Device");

  m_IGTLDeviceSource =  mitk::IGTLDeviceSource::New();
  m_IGTLDeviceSource->SetIGTLDevice(m_OpenIGTLinkClient);

}


mitk::OpenIGTLinkTrackingDevice::~OpenIGTLinkTrackingDevice()
{
}

int mitk::OpenIGTLinkTrackingDevice::GetPortNumber()
{
  return m_OpenIGTLinkClient->GetPortNumber();
}

std::string mitk::OpenIGTLinkTrackingDevice::GetHostname()
{
  return m_OpenIGTLinkClient->GetHostname();
}


void mitk::OpenIGTLinkTrackingDevice::SetPortNumber(int portNumber)
{
  m_OpenIGTLinkClient->SetPortNumber(portNumber);
}

void mitk::OpenIGTLinkTrackingDevice::SetHostname(std::string hostname)
{
  m_OpenIGTLinkClient->SetHostname(hostname);
}

mitk::TrackingTool* mitk::OpenIGTLinkTrackingDevice::AddTool( const char* toolName, const char* fileName )
{
  mitk::OpenIGTLinkTrackingTool::Pointer t;// = mitk::OpenIGTLinkTrackingTool::New();
  //TODO: Implement
  if (this->InternalAddTool(t) == false)
    return NULL;
  return t.GetPointer();
}


bool mitk::OpenIGTLinkTrackingDevice::InternalAddTool(OpenIGTLinkTrackingTool::Pointer tool)
{
  m_AllTools.push_back(tool);
  return true;
}


bool mitk::OpenIGTLinkTrackingDevice::DiscoverTools()
{
  if (m_OpenIGTLinkClient->GetPortNumber() == -1)
    {
    MITK_WARN << "Connection not initialized, aborting (invalid port number).";
    return false;
    }

  try
    {
    m_IGTLDeviceSource->Connect();
    }
  catch(std::runtime_error &e)
    {
    MITK_WARN << "Open IGT Link device retruned an error while trying to connect: " << e.what();
    return false;
    }




  //TODO: Implement

  return true;
}


bool mitk::OpenIGTLinkTrackingDevice::StartTracking()
{
  //TODO: Implement
  return false;
}


bool mitk::OpenIGTLinkTrackingDevice::StopTracking()
{
  Superclass::StopTracking();
  return true;
}


unsigned int mitk::OpenIGTLinkTrackingDevice::GetToolCount() const
{
  return (unsigned int)this->m_AllTools.size();
}


mitk::TrackingTool* mitk::OpenIGTLinkTrackingDevice::GetTool(unsigned int toolNumber) const
{
  if ( toolNumber >= this->GetToolCount())
    return NULL;
  else
    return this->m_AllTools[toolNumber];
}


bool mitk::OpenIGTLinkTrackingDevice::OpenConnection()
{
  bool returnValue = false;
  if (!m_OpenIGTLinkClient->TestConnection()) //TestConnection() not implemented yet!
    {
    MITK_WARN << "No connection available, aborting!";
    return false;
    }


  return returnValue;
}


bool mitk::OpenIGTLinkTrackingDevice::CloseConnection()
{
  bool returnValue = false;
  //TODO: Implement
  return returnValue;
}

std::vector<mitk::OpenIGTLinkTrackingTool::Pointer> mitk::OpenIGTLinkTrackingDevice::GetAllTools()
{
  return this->m_AllTools;
}


void mitk::OpenIGTLinkTrackingDevice::TrackTools()
{
  //TODO: Implement
}



ITK_THREAD_RETURN_TYPE mitk::OpenIGTLinkTrackingDevice::ThreadStartTracking(void* pInfoStruct)
{
  /* extract this pointer from Thread Info structure */
  struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
  if (pInfo == NULL)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  if (pInfo->UserData == NULL)
  {
    return ITK_THREAD_RETURN_VALUE;
  }
  OpenIGTLinkTrackingDevice *trackingDevice = (OpenIGTLinkTrackingDevice*)pInfo->UserData;

  if (trackingDevice != NULL)
    trackingDevice->TrackTools();

  return ITK_THREAD_RETURN_VALUE;
}
