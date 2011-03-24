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
#ifndef __mitkToFCameraPMDO3Controller_h
#define __mitkToFCameraPMDO3Controller_h

#include "mitkToFHardwareExports.h"
#include "mitkCommon.h"
#include "mitkToFCameraPMDController.h"

#include "itkObject.h"
#include "itkObjectFactory.h"

namespace mitk
{
  /**
  * @brief Interface to the Time-of-Flight (ToF) camera PMD O3
  * Connection is established over Ethernet connection. IP address must be specified.
  *
  * @ingroup ToFHardware
  */
  class MITK_TOFHARDWARE_EXPORT ToFCameraPMDO3Controller : public mitk::ToFCameraPMDController
  {
  public: 

    mitkClassMacro( ToFCameraPMDO3Controller , mitk::ToFCameraPMDController );

    itkNewMacro( Self );

    /*!
    \brief opens a connection to the PMD O3 ToF camera
    */
    virtual bool OpenCameraConnection();

  protected:

    ToFCameraPMDO3Controller();

    ~ToFCameraPMDO3Controller();

  private:

    char *m_IPAddress; ///< holds the ip adress the O3 camera is connected to
  };
} //END mitk namespace
#endif