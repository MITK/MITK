/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTrackingDevice.h"
#include "mitkIGTTimeStamp.h"
#include "mitkTrackingTool.h"

#include <usModuleContext.h>
#include <usGetModuleContext.h>

#include "mitkUnspecifiedTrackingTypeInformation.h"
#include "mitkTrackingDeviceTypeCollection.h"


mitk::TrackingDevice::TrackingDevice() :
  m_State(mitk::TrackingDevice::Setup),
  m_Data(mitk::UnspecifiedTrackingTypeInformation::GetDeviceDataUnspecified()),
  m_StopTracking(false),
  m_RotationMode(mitk::TrackingDevice::RotationStandard)

{
}


mitk::TrackingDevice::~TrackingDevice()
{
}

bool mitk::TrackingDevice::IsDeviceInstalled()
{
return true;
//this is the default for all tracking device
//If a device needs installation please reimplement
//this method in the subclass.
}

bool mitk::TrackingDevice::AutoDetectToolsAvailable()
{
  return false;
}

bool mitk::TrackingDevice::AddSingleToolIsAvailable()
{
  return true;
}

mitk::NavigationToolStorage::Pointer mitk::TrackingDevice::AutoDetectTools()
{
  return mitk::NavigationToolStorage::New();
}


mitk::TrackingDevice::TrackingDeviceState mitk::TrackingDevice::GetState() const
{
  std::lock_guard<std::mutex> lock(m_StateMutex);
  return m_State;
}


void mitk::TrackingDevice::SetState( TrackingDeviceState state )
{
  itkDebugMacro("setting  m_State to " << state);

  std::lock_guard<std::mutex> lock(m_StateMutex);
  if (m_State == state)
  {
    return;
  }
  m_State = state;
  this->Modified();
}

void mitk::TrackingDevice::SetRotationMode(RotationMode)
{
  MITK_WARN << "Rotation mode switching is not implemented for this device. Leaving it at mitk::TrackingDevice::RotationStandard";
}

mitk::TrackingDeviceType mitk::TrackingDevice::GetType() const{
  return m_Data.Line;
}

void mitk::TrackingDevice::SetType(mitk::TrackingDeviceType deviceType){

  us::ModuleContext* context = us::GetModuleContext();

   std::vector<us::ServiceReference<mitk::TrackingDeviceTypeCollection> > refs = context->GetServiceReferences<mitk::TrackingDeviceTypeCollection>();
   if (refs.empty())
   {
     MITK_ERROR << "No tracking device service found!";
   }
   mitk::TrackingDeviceTypeCollection* deviceTypeCollection = context->GetService<mitk::TrackingDeviceTypeCollection>(refs.front());

   m_Data = deviceTypeCollection->GetFirstCompatibleDeviceDataForLine(deviceType);
}

mitk::TrackingDeviceData mitk::TrackingDevice::GetData() const{
  return m_Data;
}


void mitk::TrackingDevice::SetData(mitk::TrackingDeviceData data){
  m_Data = data;
}


bool mitk::TrackingDevice::StopTracking()
{
  if (this->GetState() == Tracking) // Only if the object is in the correct state
  {
    m_StopTrackingMutex.lock();  // m_StopTracking is used by two threads, so we have to ensure correct thread handling
    m_StopTracking = true;
    m_StopTrackingMutex.unlock();
    //we have to wait here that the other thread recognizes the STOP-command and executes it
    m_TrackingFinishedMutex.lock();
    mitk::IGTTimeStamp::GetInstance()->Stop(this); // notify realtime clock
    // StopTracking was called, thus the mode should be changed back
    //   to Ready now that the tracking loop has ended.
    this->SetState(Ready);
    m_TrackingFinishedMutex.unlock();
  }
  return true;
}


mitk::TrackingTool* mitk::TrackingDevice::GetToolByName( std::string name ) const
{
  unsigned int toolCount = this->GetToolCount();
  for (unsigned int i = 0; i < toolCount; ++i)
    if (name == this->GetTool(i)->GetToolName())
      return this->GetTool(i);
  return nullptr;
}

std::string mitk::TrackingDevice::GetTrackingDeviceName()
{
  return this->GetData().Line;
}
