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

#ifndef MITKWINDOWSREALTIMECLOCK_H_HEADER_INCLUDED_
#define MITKWINDOWSREALTIMECLOCK_H_HEADER_INCLUDED_

#include "mitkRealTimeClock.h"

namespace mitk {

  /**Documentation
  * @brief realtimeclock implementation for windows-systems
  *
  * This class provides a realtimeclock for windows-systems.
  * It uses the QueryPerformanceCounter and the QueryPerformaceFrequency.
  * It polls the current tick-counter, (that counts from bootup ?!?)
  * is supposed to be the most accurate time you can get on a windows-system.
  *
  *@ingroup Navigation
  */

  class WindowsRealTimeClock : public RealTimeClock
  {
  public:
    mitkClassMacro(WindowsRealTimeClock, mitk::RealTimeClock);
    itkNewMacro(Self);

    /**
    * \brief basic contructor
    */
    WindowsRealTimeClock();
    
    /**
    * \brief basic destructor
    */
    virtual ~WindowsRealTimeClock();

    /**
    * \brief returns the current time in milliseconds as a double 
    */
    virtual double getCurrentStamp();

    /**
    * \brief returns the QueryPerformanceFrequency
    */
    virtual LARGE_INTEGER getFrequency();
    
  protected:

    LARGE_INTEGER frequency;

  }; //namespace
}






#endif /* MITKWINDOWSREALTIMECLOCK_H_HEADER_INCLUDED_ */