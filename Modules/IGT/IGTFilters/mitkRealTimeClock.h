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

#ifndef MITKWREALTIMECLOCK_H_HEADER_INCLUDED_
#define MITKWREALTIMECLOCK_H_HEADER_INCLUDED_


#include <itkObject.h>
#include <itkObjectFactory.h>
#include "MitkIGTExports.h"
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

  class MitkIGT_EXPORT RealTimeClock : public itk::Object
  {
  public:
    mitkClassMacro(RealTimeClock, itk::Object);

    /**
    *\brief instanciates a new, operating-system dependant, instance of mitk::RealTimeClock.
    */
    static Pointer New(void);

    virtual double GetCurrentStamp() = 0;
  };
} //namespace
#endif /* MITKWREALTIMECLOCK_H_HEADER_INCLUDED_ */
