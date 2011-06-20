/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2010-05-27 16:06:53 +0200 (Do, 27 Mai 2010) $
Version:   $Revision:  $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __mitkToFCameraMESASR4000Controller_h
#define __mitkToFCameraMESASR4000Controller_h

#include "mitkToFHardwareExports.h"
#include "mitkCommon.h"
#include "mitkToFCameraMESAController.h"

#include "itkObject.h"
#include "itkObjectFactory.h"

namespace mitk
{
  /**
  * @brief Interface to the Time-of-Flight (ToF) camera MESA Swissranger 4000
  * 
  *
  * @ingroup ToFHardware
  */
  class MITK_TOFHARDWARE_EXPORT ToFCameraMESASR4000Controller : public mitk::ToFCameraMESAController
  {
  public: 

    mitkClassMacro( ToFCameraMESASR4000Controller , mitk::ToFCameraMESAController );

    itkNewMacro( Self );

    /*!
    \brief opens a connection to the ToF camera and initializes the hardware specific members
    \return returns whether the connection was successful (true) or not (false)
    */
    virtual bool OpenCameraConnection();

  protected:

    ToFCameraMESASR4000Controller();

    ~ToFCameraMESASR4000Controller();

  private:

  };
} //END mitk namespace
#endif
