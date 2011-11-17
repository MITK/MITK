/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkTimeStamp.h"
#include <map>

#include "mitkRealTimeClock.h"

mitk::TimeStamp::Pointer mitk::TimeStamp::s_Instance = NULL;

mitk::TimeStamp::TimeStamp() : itk::Object()
, m_Time(-1.0), m_ReferenceTime(0.0)
{
}

mitk::TimeStamp::~TimeStamp()
{
}

mitk::TimeStamp* mitk::TimeStamp::CreateInstance()
{
return mitk::TimeStamp::GetInstance();
}

mitk::TimeStamp* mitk::TimeStamp::GetInstance()
{
  if (TimeStamp::s_Instance.IsNull())
  {
    mitk::TimeStamp::Pointer ts = new mitk::TimeStamp;
    s_Instance = ts;
    return s_Instance;
  }
  else  
    return s_Instance;
}

void mitk::TimeStamp::Start(itk::Object::Pointer device)
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

void mitk::TimeStamp::Stop(itk::Object::Pointer device)
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
      m_ReferenceTime = NULL;
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


double mitk::TimeStamp::GetElapsed()
{
  if (m_Time > -1)
  {
    m_Time = GetCurrentStamp();
    m_Time = m_Time - m_ReferenceTime;
  }
  return (double) m_Time;
}


double mitk::TimeStamp::GetElapsed(itk::Object::Pointer device)
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

double mitk::TimeStamp::GetCurrentStamp()
{
  if (m_RealTimeClock.IsNotNull())
  {
    return m_RealTimeClock->GetCurrentStamp();
  }
  else return 0.0;
}

void mitk::TimeStamp::SetRealTimeClock(mitk::RealTimeClock::Pointer Clock)
{
  m_RealTimeClock = Clock;
}

double mitk::TimeStamp::GetOffset(itk::Object::Pointer Device)
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

void mitk::TimeStamp::Initialize()
{
  if ( m_RealTimeClock.IsNull() )
    m_RealTimeClock = mitk::RealTimeClock::New();
}

