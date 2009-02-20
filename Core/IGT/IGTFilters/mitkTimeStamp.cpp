/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
, m_Time(0.0), m_Devices(0)
{
}

mitk::TimeStamp::~TimeStamp()
{
}

const mitk::TimeStamp* mitk::TimeStamp::GetTimeStamp()
{
  return new TimeStamp();  // \TODO Access real time clock to get meaningful timestamp
}


mitk::TimeStamp* mitk::TimeStamp::CreateInstance()
{
  if ( s_Instance.IsNotNull() )
  {

    return s_Instance;
  }
  try
  {
    mitk::TimeStamp::Pointer ts = new mitk::TimeStamp;
    s_Instance = ts;
  }
  catch(...)
  {
    s_Instance = NULL;
    throw 1;
  }
  return s_Instance;

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

void mitk::TimeStamp::StartTracking(itk::Object::Pointer Device)
{
  if ( s_Instance.IsNotNull() )
  {
    if (m_DeviceMap.empty())
    {
      m_ReferenceTime = GetCurrentStamp();
      m_Time = 0.0;
    } 
    m_DeviceMap.insert( std::pair<itk::Object::Pointer, double>(Device, this->GetElapsed()) );
  }
  else
  {
    itkGenericOutputMacro("Trying to use mitk::TimeStamp::StartTracking() " 
        << "without an available singleton instance. Either no instance has "
        << "been created (use TimeStamp::CreateInstance) or it has already "
        << "been destroyed.");
  }
}             

void mitk::TimeStamp::StopTracking(itk::Object::Pointer Device)
{
  if ( s_Instance.IsNotNull() )
  {
    m_MapIterator =  m_DeviceMap.find(Device);
    if ( m_MapIterator != m_DeviceMap.end() )
    {
      m_DeviceMap.erase( m_MapIterator );
    }
    
    if (m_DeviceMap.empty())
    {
      m_ReferenceTime = NULL;
      m_Time = NULL;
    }
  }
  else
  {
    itkGenericOutputMacro("Trying to use mitk::TimeStamp::StopTracking() " 
        << "without an available singleton instance. Either no instance has "
        << "been created (use TimeStamp::CreateInstance) or it has already "
        << "been destroyed.");    
  }
}

double mitk::TimeStamp::GetElapsed()
{
  m_Time = GetCurrentStamp();
  m_Time = m_Time - m_ReferenceTime;
  return (double) m_Time;
}


double mitk::TimeStamp::GetCurrentStamp()
{
//#  if defined (WIN32) || defined (_WIN32)
//  TimeVal counts, time;
//  QueryPerformanceCounter( (LARGE_INTEGER*)&counts );
//  time = (counts * 100000) / Frequency.QuadPart;
//  double milliseconds = (double) (time & 0xffffffff);
//  (double) milliseconds /= 100.0;
//  return milliseconds;
//#else
//
//  struct double tval;
//
//  ::gettimeofday( &tval, 0 );
//
//  double milliseconds = static_cast< double >( tval.tv_sec ) +
//          static_cast< double >( tval.tv_usec ) / Frequency;
//
//  return milliseconds*1000; // in milliseconds
//
//#endif  // defined(WIN32) || defined(_WIN32)

  return m_RealTimeClock->getCurrentStamp();
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

