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
//#include "mitkTrackingDevice.h"

//namespace Poco {
//  class Timestamp;
//}

namespace mitk {

  /**Documentation
  * @brief Time stamp in milliseconds
  *
  * This class provides a timestamp in milliseconds.
  *  WORK IN PROGRESS
  *
  *@ingroup Navigation
  */
  class TimeStamp : public itk::Object
  {
  public:
    
    mitkClassMacro(TimeStamp, itk::Object);
    //itkNewMacro(Self);
    
    static const TimeStamp* GetTimeStamp();

    static TimeStamp* CreateInstance();

    static TimeStamp* GetInstance();

    void StartTracking( itk::Object::Pointer Device );

    void StopTracking( itk::Object::Pointer Device );

    double GetElapsed();

    double GetOffset(itk::Object::Pointer Device);

    void SetRealTimeClock(mitk::RealTimeClock::Pointer Clock);
 
  protected:
    TimeStamp();

    virtual ~TimeStamp();

    double GetCurrentStamp();

    double m_Time;

    double m_ReferenceTime;

    int m_Devices;

    mitk::RealTimeClock::Pointer m_RealTimeClock;

    static mitk::TimeStamp::Pointer s_Instance;

    std::map<itk::Object::Pointer, double> m_DeviceMap;

    std::map<itk::Object::Pointer, double>::iterator m_MapIterator;    
  };
} // namespace mitk

#endif /* MITKTIMESTAMP_H_HEADER_INCLUDED_ */
