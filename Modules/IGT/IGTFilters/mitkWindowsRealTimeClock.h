/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "MitkIGTExports.h"
#include "mitkRealTimeClock.h"


namespace mitk {

  /**
  * \brief realtimeclock implementation for windows-systems
  *
  * This class provides a RealTimeClock for windows-systems.
  * Internally, it uses the QueryPerformanceCounter and the QueryPerformaceFrequency.
  * It polls the current tick-counter, that counts from bootup.
  * is supposed to be the most accurate time you can get on a windows-system.
  *
  * \ingroup Navigation
  */

  class MitkIGT_EXPORT WindowsRealTimeClock : public RealTimeClock
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
    *
    * Uses the QueryPerformanceCounter to acquire an accurate time.
    * (Ticks counted / Frequency) = time in ms
    */
    virtual double GetCurrentStamp();

    /**
    * \brief returns the QueryPerformanceFrequency
    */
    virtual LARGE_INTEGER GetFrequency();
    
  protected:

    void SetFrequency();

    /**
    * \brief Frequency needed to calculate time from tick-counter
    */
    LARGE_INTEGER m_Frequency;

  }; //namespace
}






#endif /* MITKWINDOWSREALTIMECLOCK_H_HEADER_INCLUDED_ */