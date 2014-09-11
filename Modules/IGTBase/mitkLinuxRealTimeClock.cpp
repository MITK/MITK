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

#include "mitkLinuxRealTimeClock.h"

#include <sys/time.h>


/**
* \brief basic constructor
*/
mitk::LinuxRealTimeClock::LinuxRealTimeClock()
{
}

/**
* \brief basic destructor
*/
mitk::LinuxRealTimeClock::~LinuxRealTimeClock()
{
}

/**
* \brief returns the current time in milliseconds
*
* Gets the current time since the Epoch (01.01.1970). gettimeofday returns the elapsed time in
* microseconds.This value is calculated to milliseconds and returned as a double.
*
* \return Returns the elapsed time in milliseconds
*/
double mitk::LinuxRealTimeClock::GetCurrentStamp()
{
  struct timeval tval;

  if ( ::gettimeofday( &tval, 0 )!= 0 )
  {
    itkGenericOutputMacro("gettimeofday-method could not successfully acquire the current time");
    return -1;
  }
  double milliseconds;

  milliseconds = static_cast< double >( tval.tv_sec ) +
    static_cast< double >( tval.tv_usec ) / 1e6;

  return milliseconds*1000; // in milliseconds
}
