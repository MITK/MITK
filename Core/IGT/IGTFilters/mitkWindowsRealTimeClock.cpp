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

#include "mitkWindowsRealTimeClock.h"

#include "windows.h"


/**
* \brief basic contructor 
*/
mitk::WindowsRealTimeClock::WindowsRealTimeClock()
{
  SetFrequency();
}

/**
* \brief basic contructor 
*/
mitk::WindowsRealTimeClock::~WindowsRealTimeClock()
{

}

void mitk::WindowsRealTimeClock::SetFrequency()
{
  if ( !QueryPerformanceFrequency(&m_Frequency) )
  {
    m_Frequency.QuadPart = 0;
  }
}

/**
* \brief returns the current time in milliseconds as a double
*/
double mitk::WindowsRealTimeClock::GetCurrentStamp()
{
// "if defined" not really necessary in this case, as the class is only available on Windows-systems
  __int64 time, ticks = 0;

  if (m_Frequency.QuadPart < 1)
  {
    return -1.0;
  }

  QueryPerformanceCounter( (LARGE_INTEGER*) &ticks);
  time = (ticks * 100000) / this->m_Frequency.QuadPart;
  double milliseconds = (double) (time & 0xffffffff);
  (double) milliseconds /= 100.0;
  return milliseconds;
}

/**
* \brief returns the QueryPerformanceFrequency, needed for acquiring the time from ticks
*/
LARGE_INTEGER mitk::WindowsRealTimeClock::GetFrequency()
{
  return this->m_Frequency;
}



