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

#ifndef MITKLINUXREALTIMECLOCK_H_HEADER_INCLUDED_
#define MITKLINUXREALTIMECLOCK_H_HEADER_INCLUDED_

#include "mitkRealTimeClock.h"

namespace mitk {

  /**
  * \brief RealTimeClock for linux-systems
  *
  * This class provides realtimeclock for linux-systems.
  * It uses gettimeofday to acquire a current time-value as accurately
  * as possible.
  *
  * \return Returns the current time since the Epoch (01.01.1970) in milliseconds as a double
  *
  * \ingroup IGT
  */

  class MitkIGT_EXPORT LinuxRealTimeClock : public RealTimeClock
  {
  public:
    mitkClassMacro(mitk::LinuxRealTimeClock, mitk::RealTimeClock);
    itkNewMacro(Self);

    /**
    * \brief basic contructor
    */
    LinuxRealTimeClock();

    /**
    * \brief basic contructor
    */
    virtual ~LinuxRealTimeClock();

    /**
    * \brief returns the current time in milliseconds as a double
    */
    virtual double GetCurrentStamp();

  }; //namespace
}

#endif /* MITKLINUXREALTIMECLOCK_H_HEADER_INCLUDED_ */
