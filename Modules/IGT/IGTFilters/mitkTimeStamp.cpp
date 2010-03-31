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


/**
* \brief creates a new instance of mitkTimeStamp
*
* if no instance if mitkTimeStamp exisits, a new one is created and returned, else
* simply the existing one is returned
* If a new instance could not be created, 1 is thrown and NULL is returned.
*/
mitk::TimeStamp* mitk::TimeStamp::CreateInstance()
{
  if ( s_Instance.IsNotNull() )
  {
    return s_Instance;
  }
  else
  {
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

}

/**
* \brief returns a pointer to the current instance of mitkTimeStamp
*
* This method  returns a pointer to the currently existing TimeStamp.
* If there is no exisiting instance, a new one is created and returned automatically
*/
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

/**
* \brief this method starts the time-acquiring
*
* This method first checks if a RealTimeClock has been set. If not, initialize() is called
* After this, a map is searched if any device has already "started tracking". Starting twice does not
* make sense and is not allowed. 
* An internal variable saves the current time as a reference-time. All returned times are relative to this 
* starting-time.
* Then the calling device is added to the map of itk::Objects.
*/
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

/**
* \brief this method ends the time-acquisition for a device
*
* When this method is called, the device that called will be erased from the map
* of devices. When the map is empty, the reference-time and the current timestamp 
* are reset to 0.
*/
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


/**
* \brief returns the current time acquired from the defined RealTimeClock
*
* only used internally
*/
double mitk::TimeStamp::GetCurrentStamp()
{
  if (m_RealTimeClock.IsNotNull())
  {
    return m_RealTimeClock->GetCurrentStamp();
  }
  else return 0.0;
}

/**
* \brief setter for the internally used RealTimeClock
*
* If you want to use a "third-party" RealTimeClock, e.g PocoRealTimeClock, BoostRealTimeClock
* or ITKRealTimeClock, you can set it using this method:
* mitk::<bla>RealTimeClock::Pointer RealTimeClock = mitk::<bla>RealTimeClock::New();
* mitk::TimeStamp::GetInstance()->SetRealTimeClock(RealTimeClock);
*
* Notice: The mitk-implementation of an os-dependant RealTimeClock is used 
* by default.
*/
void mitk::TimeStamp::SetRealTimeClock(mitk::RealTimeClock::Pointer Clock)
{
  m_RealTimeClock = Clock;
}

/**
* 
*
* This method returns the time acquired when Start was called.
* This is the offset, that each device will have in relation to the device that 
* started the time acqusition, that means, called Start first.
* Thus absolute time-values can be calculated for each device by subtracting the offset 
* of all timeStamps.
*
* If this device has not been or is no longer saved in the map of devices,
* -1 will be returned.
*
*/
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

/**
* \brief creates a new RealTimeClock
*
* Instanciates a new RealTimeClock, that will be specific for the Operating System.
* This will only be called internally when no other RealTimeClock has been set
* by the user.
*
*/
void mitk::TimeStamp::Initialize()
{
  if ( m_RealTimeClock.IsNull() )
    m_RealTimeClock = mitk::RealTimeClock::New();
}

