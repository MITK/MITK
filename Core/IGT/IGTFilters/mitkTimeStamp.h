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


#ifndef MITKTIMESTAMP_H_HEADER_INCLUDED_
#define MITKTIMESTAMP_H_HEADER_INCLUDED_

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>

#include "mitkRealTimeClock.h"


namespace mitk {

  /**
  * \brief Time stamp in milliseconds
  *
  * This class provides a timestamp in milliseconds.
  * It is a Singleton class, that internally uses a mitkRealTimeClock() for
  * time-acquisition. 
  *
  * First you have to call StartTracking() in order to set the reference-time to the current time.
  * If the user has not created and set his own "RealTimeClock", initialize() will be called and a
  * default mitkRealTimeClock() is created.
  * In addition the TimeStamp() saves a pointer to the device calling and the respective offset-time.
  * The first device will have an offset of 0, the following's offset will be the time elapsed since the 
  * starting of the first device. This offset can be prompted by calling GetOffset();
  *
  * You can always get the time elapsed since calling StartTracking() with GetElapsed(). It returns the 
  * time spent in milliseconds as a double.
  *
  * When the TimeStamp is no longer used, you can call StopTracking(). This erases the pointer to the device 
  * and the offset. When all devices have "stopped tracking" the reference-time and the current-time are reset to 0.
  *
  * \ingroup IGT
  */
  class TimeStamp : public itk::Object
  {
  public:
    
    mitkClassMacro(TimeStamp, itk::Object);
    
    /**
    * \brief creates and returns a new instance of mitkTimeStamp if necessary
    */
    static TimeStamp* CreateInstance();

    /**
    * \brief returns the current instance of mitkTimeStamp if one exists.
    *
    * If no instance exists, a new one is created.
    */
    static TimeStamp* GetInstance();

    /**
    * \brief starts the time-acquisition
    *
    * Each device is to call this method when it starts tracking.
    * The current time is saved as a reference-value (m_Time = 0).
    * Internally the device (pointer) and its offset are saved in a map, so that 
    * no device can call this method twice.
    * If the user has not set its own RealTimeClock, a default one is created dependant on the OS
    * in use.
    *
    */
    void StartTracking( itk::Object::Pointer Device );

    /**
    * \brief stops the time-acqusition
    *
    * Each device has to call StopTracking() when it has finished and its
    * pointer will be erased from the map. When the last device has "stopped"
    * the reference-time and the current-time will be reset to 0.
    *
    */
    void StopTracking( itk::Object::Pointer Device );

    /**
    * \brief returns the time elapsed since calling StartTracking() for the first time in milliseconds
    *
    */
    double GetElapsed();

    /**
    * \brief returns the offset of this device's starting-time to the 
    *  reference-time in ms
    *
    * Device 'A' is the first device to call StartTracking. Device 'B' calls StartTracking
    * some time later. This time-difference is the offset, that each device has realtive to the 
    * device that started the time-acquisition.
    * Each device's offset is stored in a map with a pointer to the device.
    *
    * If this device has not been or is no longer saved in the map of devices,
    * -1 will be returned.
    *
    *
    * only used internally
    */
    double GetOffset(itk::Object::Pointer Device);

    /**
    * \brief setter for the internally used RealTimeClock()
    *
    * If you want to use a "third-party" RealTimeClock, e.g PocoRealTimeClock, BoostRealTimeClock
    * or ITKRealTimeClock, you can set it using this method:
    * mitk::<bla>RealTimeClock::Pointer RealTimeClock = mitk::<bla>RealTimeClock::New();
    * mitk::TimeStamp::GetInstance()->SetRealTimeClock(RealTimeClock);
    *
    * Right now, none of these RealTimeClocks have been implemented!!
    *
    * Notice: The mitk-implementation of an os-dependant RealTimeClock is used 
    * by default.
    */
    void SetRealTimeClock(mitk::RealTimeClock::Pointer Clock);
 
  protected:
    TimeStamp();

    virtual ~TimeStamp();

    double GetCurrentStamp();

    void Initialize();

    /*
      the current timestamp when GetCurrentStamp() is called.
    */
    double m_Time;

    /*
      the timestamp in ms acquired when StartTracking() was called.
    */
    double m_ReferenceTime;

    /*
      pointer to the RealTimeClock used internally
    */
    mitk::RealTimeClock::Pointer m_RealTimeClock;

    /*
      pointer to the current instance
    */
    static mitk::TimeStamp::Pointer s_Instance;

    /*
      map, in which pointer to all devices calling StartTracking(), are saved
    */
    std::map<itk::Object::Pointer, double> m_DeviceMap;

    std::map<itk::Object::Pointer, double>::iterator m_MapIterator;    
  };
} // namespace mitk

#endif /* MITKTIMESTAMP_H_HEADER_INCLUDED_ */
