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

#include "mitkRealTimeClock.h"


// check if MITK runs on a Windows-System
#ifdef _WIN32
  #include "mitkWindowsRealTimeClock.h"
#else // should be Linux or macOS
  #include "mitkLinuxRealTimeClock.h"
#endif

mitk::RealTimeClock::Pointer mitk::RealTimeClock::New()
{
  mitk::RealTimeClock::Pointer smartPtr;

#ifdef _WIN32
  smartPtr = mitk::WindowsRealTimeClock::New();
#else
  smartPtr = mitk::LinuxRealTimeClock::New();
#endif

  return smartPtr;
}
