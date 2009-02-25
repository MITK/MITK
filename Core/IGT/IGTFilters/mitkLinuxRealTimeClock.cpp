/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-02-10 18:08:54 +0100 (Di, 10 Feb 2009) $
Version:   $Revision: 16228 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
double mitk::LinuxRealTimeClock::getCurrentStamp()
{
  struct timeval tval;

  ::gettimeofday( &tval, 0 );
  double milliseconds;

  milliseconds = static_cast< double >( tval.tv_sec ) +
          static_cast< double >( tval.tv_usec ) / 1e6;

  return milliseconds*1000; // in milliseconds
}

