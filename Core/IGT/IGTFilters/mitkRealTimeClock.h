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

#ifndef MITKWREALTIMECLOCK_H_HEADER_INCLUDED_
#define MITKWREALTIMECLOCK_H_HEADER_INCLUDED_


#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>


namespace mitk {

  /**Documentation
  * \brief RealTimeClock is a superclass to WindowsRealTimeClock, LinuxRealTimeClock, etc.
  *
  * This class provides the basic interface, that all RealTimeClocks have in common.
  * 
  *
  * \ingroup IGT
  */

  class RealTimeClock : public itk::Object
  {
  public:
    mitkClassMacro(RealTimeClock, itk::Object);

    RealTimeClock();

    virtual ~RealTimeClock();

    virtual double GetCurrentStamp() = 0;

  }; //namespace
}

#endif /* MITKWREALTIMECLOCK_H_HEADER_INCLUDED_ */
