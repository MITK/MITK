/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKLINUXREALTIMECLOCK_H_HEADER_INCLUDED_
#define MITKLINUXREALTIMECLOCK_H_HEADER_INCLUDED_

#include "mitkRealTimeClock.h"
#include "MitkIGTBaseExports.h"

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

  class MITKIGTBASE_EXPORT LinuxRealTimeClock : public RealTimeClock
  {
  public:
    mitkClassMacro(mitk::LinuxRealTimeClock, mitk::RealTimeClock);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
    * \brief basic contructor
    */
    LinuxRealTimeClock();

    /**
    * \brief basic contructor
    */
    ~LinuxRealTimeClock() override;

    /**
    * \brief returns the current time in milliseconds as a double
    */
    double GetCurrentStamp() override;

  }; //namespace
}

#endif /* MITKLINUXREALTIMECLOCK_H_HEADER_INCLUDED_ */
