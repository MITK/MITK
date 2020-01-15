/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIGTTimeStamp.h"
#include <map>

#include "mitkRealTimeClock.h"

mitk::IGTTimeStamp::Pointer mitk::IGTTimeStamp::s_Instance = nullptr;

mitk::IGTTimeStamp::IGTTimeStamp() : itk::Object()
, m_Time(-1.0), m_ReferenceTime(0.0)
{
}

mitk::IGTTimeStamp::~IGTTimeStamp()
{
}

mitk::IGTTimeStamp* mitk::IGTTimeStamp::CreateInstance()
{
return mitk::IGTTimeStamp::GetInstance();
}

mitk::IGTTimeStamp* mitk::IGTTimeStamp::GetInstance()
{
  if (IGTTimeStamp::s_Instance.IsNull())
  {
    mitk::IGTTimeStamp::Pointer ts = new mitk::IGTTimeStamp;
    s_Instance = ts;
    return s_Instance;
  }
  else
    return s_Instance;
}

void mitk::IGTTimeStamp::Start(itk::Object::Pointer device)
{
  if (m_RealTimeClock.IsNull())
  {
    Initialize();
  }
  if ( s_Instance.IsNotNull() )
  {
    if (m_DeviceMap.empty())
    {
      m_ReferenceTime = GetCurrentStamp();
      m_Time = 0.0;
    }
    m_DeviceMap.insert( std::pair<itk::Object::Pointer, double>(device, this->GetElapsed()) );
  }
  else
  {
    itkGenericOutputMacro("Trying to use mitk::TimeStamp::Start() "
        << "without an available singleton instance. Either no instance has "
        << "been created (use TimeStamp::CreateInstance) or it has already "
        << "been destroyed.");
  }
}

void mitk::IGTTimeStamp::Stop(itk::Object::Pointer device)
{
  if ( s_Instance.IsNotNull() )
  {
    m_MapIterator =  m_DeviceMap.find(device);
    if ( m_MapIterator != m_DeviceMap.end() )
    {
      m_DeviceMap.erase( m_MapIterator );
    }

    if (m_DeviceMap.empty())
    {
      m_ReferenceTime = 0;
      m_Time = -1;
    }
  }
  else
  {
    itkGenericOutputMacro("Trying to use mitk::TimeStamp::Stop() "
        << "without an available singleton instance. Either no instance has "
        << "been created (use TimeStamp::CreateInstance) or it has already "
        << "been destroyed.");
  }
}


double mitk::IGTTimeStamp::GetElapsed()
{
  if (m_Time > -1)
  {
    m_Time = GetCurrentStamp();
    m_Time = m_Time - m_ReferenceTime;
  }
  return (double) m_Time;
}


double mitk::IGTTimeStamp::GetElapsed(itk::Object::Pointer device)
{
  double offset = this->GetOffset( device );
  if ( offset > -1 )
  {
    double time = this->GetElapsed();
    return (double) time - this->GetOffset(device);
  }
  else
  {
    return (double) -1;
  }
}

double mitk::IGTTimeStamp::GetCurrentStamp()
{
  if (m_RealTimeClock.IsNotNull())
  {
    return m_RealTimeClock->GetCurrentStamp();
  }
  else return 0.0;
}

void mitk::IGTTimeStamp::SetRealTimeClock(mitk::RealTimeClock::Pointer Clock)
{
  m_RealTimeClock = Clock;
}

double mitk::IGTTimeStamp::GetOffset(itk::Object::Pointer Device)
{
  m_MapIterator =  m_DeviceMap.find(Device);
  if ( m_MapIterator != m_DeviceMap.end() )
  {
    return m_MapIterator->second;
  }
  else
  {
    return -1.0;
  }
}

void mitk::IGTTimeStamp::Initialize()
{
  if ( m_RealTimeClock.IsNull() )
    m_RealTimeClock = mitk::RealTimeClock::New();
}
