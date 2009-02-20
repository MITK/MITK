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

#ifndef MITKLINUXREALTIMECLOCK_H_HEADER_INCLUDED_
#define MITKLINUXREALTIMECLOCK_H_HEADER_INCLUDED_

#include "mitkRealTimeClock.h"

namespace mitk {

  /**Documentation
  * @brief Time stamp in milliseconds
  *
  * This class provides a timestamp in milliseconds.
  *  WORK IN PROGRESS
  *
  *@ingroup Navigation
  */

  class LinuxRealTimeClock : public RealTimeClock
  {
  public:
    mitkClassMacro(mitk::LinuxRealTimeClock, mitk::RealTimeClock);
    itkNewMacro(Self);

    LinuxRealTimeClock();

    virtual ~LinuxRealTimeClock();

    virtual double getCurrentStamp();

  }; //namespace
}

#endif /* MITKLINUXREALTIMECLOCK_H_HEADER_INCLUDED_ */
