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


#ifndef MITKTIMESTAMP_H_HEADER_INCLUDED_
#define MITKTIMESTAMP_H_HEADER_INCLUDED_

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <MitkIGTExports.h>

#include "mitkRealTimeClock.h"


namespace mitk {

  /**
  * \brief Time stamp in milliseconds
  *
  * This class provides a timestamp in milliseconds.
  * It is a Singleton class, that internally uses a mitkRealTimeClock() for
  * time-acquisition. 
  *
  * First you have to call Start() in order to set the reference-time to the current time.
  * If the user has not created and set his own "RealTimeClock", initialize() will be called and a
  * default mitkRealTimeClock() is created.
  * In addition the TimeStamp() saves a pointer to the device calling and the respective offset-time.
  * The first device will have an offset of 0, the following's offset will be the time elapsed since the 
  * starting of the first device. This offset can be prompted by calling GetOffset();
  *
  * You can always get the time elapsed since calling Start() with GetElapsed(). It returns the 
  * time spent in milliseconds as a double.
  *
  * When the TimeStamp is no longer used, you can call Stop(). This erases the pointer to the device 
  * and the offset. When all devices have "stopped tracking" the reference-time and the current-time are reset to 0.
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT TimeStamp : public itk::Object
  {
  public:
    
    mitkClassMacro(TimeStamp, itk::Object);
    
   /**
    * \brief creates a new instance of mitkTimeStamp
    *
    * This method  returns a pointer to the currently existing TimeStamp.
    * If there is no exisiting instance, a new one is created and returned automatically
    *
    * DECREPATED: Use GetInstance instead
    */
    static TimeStamp* CreateInstance();

   /**
    * \brief returns a pointer to the current instance of mitkTimeStamp
    *
    * This method  returns a pointer to the currently existing TimeStamp.
    * If there is no exisiting instance, a new one is created and returned automatically
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
    void Start( itk::Object::Pointer device );

    /**
    * \brief stops the time-acqusition
    *
    * Each device has to call Stop() when it has finished and its
    * pointer will be erased from the map. When the last device has "stopped"
    * the reference-time and the current-time will be reset to 0.
    *
    */
    void Stop( itk::Object::Pointer device );

    /**
    * \brief returns the time elapsed since calling Start() for the first time in milliseconds
    *
    * GetElapsed() returns the time elapsed since Start() has been called first, no matter
    * which itk::Object did the call. 
    * This method-call can be used if you want to need to have several processes you want to 
    * monitor and need timestamps in the same space of time, e.g. when using two tracking-devices
    * on the same experiment.
    */
    double GetElapsed();

    /**
    * \brief returns the time elapsed since 'device' called Start() in milliseconds
    *
    * GetElapsed(itk::Object device) returns the time elapsed since the given itk::Object called 
    * Start(). 
    * This overloaded method should be used when you only have one independent process to keep 
    * track of, e.g. when you want to measure how long it takes to execute a piece of code.
    */
    double GetElapsed(itk::Object::Pointer device);


    /**
    * \brief returns the offset of this device's starting-time to the 
    *  reference-time in ms
    *
    * Device 'A' is the first device to call Start(). Device 'B' calls Start()
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
 
    /**
    * \brief creates a new RealTimeClock
    *
    * Instanciates a new RealTimeClock, that will be specific for the Operating System.
    * This will only be called internally when no other RealTimeClock has been set
    * by the user.
    *
    */
    void Initialize();

  protected:

    TimeStamp();

    virtual ~TimeStamp();

    double GetCurrentStamp();

    /* the current timestamp when GetCurrentStamp() is called. */
    double m_Time;

    /* the timestamp in ms acquired when Start() was called. */
    double m_ReferenceTime;

    /* pointer to the RealTimeClock used internally */
    mitk::RealTimeClock::Pointer m_RealTimeClock;

    /* pointer to the current instance */
    static mitk::TimeStamp::Pointer s_Instance;

    /* map, in which pointer to all devices calling Start(), are saved */
    std::map<itk::Object::Pointer, double> m_DeviceMap;

    std::map<itk::Object::Pointer, double>::iterator m_MapIterator;    
  };
} // namespace mitk

#endif /* MITKTIMESTAMP_H_HEADER_INCLUDED_ */
