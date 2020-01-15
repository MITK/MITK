/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKWREALTIMECLOCK_H_HEADER_INCLUDED_
#define MITKWREALTIMECLOCK_H_HEADER_INCLUDED_


#include <itkObject.h>
#include <itkObjectFactory.h>
#include "MitkIGTBaseExports.h"
#include "mitkCommon.h"

namespace mitk {

  /**Documentation
  * \brief RealTimeClock is a superclass to WindowsRealTimeClock, LinuxRealTimeClock, etc.
  *
  * When mitk::RealTimeClock::New() is called, a new instance of a RealTimeClock will be
  * instantiated. WindowsRealTimeClock on a Windows-system and a LinuxRealTimeClock on a
  * Linux-system
  *
  * \ingroup IGT
  */

  class MITKIGTBASE_EXPORT RealTimeClock : public itk::Object
  {
  public:
    mitkClassMacroItkParent(RealTimeClock, itk::Object);

    /**
    *\brief instanciates a new, operating-system dependant, instance of mitk::RealTimeClock.
    */
    static Pointer New(void);

    virtual double GetCurrentStamp() = 0;
  };
} //namespace
#endif /* MITKWREALTIMECLOCK_H_HEADER_INCLUDED_ */
