/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
