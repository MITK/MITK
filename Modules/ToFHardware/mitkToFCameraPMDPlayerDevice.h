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
#ifndef __mitkToFCameraPMDPlayerDevice_h
#define __mitkToFCameraPMDPlayerDevice_h

#include "mitkToFHardwareExports.h"
#include "mitkCommon.h"
#include "mitkToFCameraDevice.h"
#include "mitkToFCameraPMDDevice.h"

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkMultiThreader.h"
#include "itkFastMutexLock.h"


namespace mitk
{
  /**
  * @brief Device class representing a player for PMD data
  *
  *
  * @ingroup ToFHardware
  */
  class MITK_TOFHARDWARE_EXPORT ToFCameraPMDPlayerDevice : public ToFCameraPMDDevice
  {
  public: 

    mitkClassMacro( ToFCameraPMDPlayerDevice , ToFCameraPMDDevice );

    itkNewMacro( Self );


    /*!
    \brief set a BaseProperty
    */
    virtual void SetProperty(const char* propertyKey, BaseProperty* propertyValue);

  protected:

    ToFCameraPMDPlayerDevice();

    ~ToFCameraPMDPlayerDevice();
    std::string m_InputFileName; ///< input file name for data stored in an .pmd file

  private:

  };
} //END mitk namespace
#endif
