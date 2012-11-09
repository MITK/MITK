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
#ifndef __mitkToFCameraPMDMITKPlayerDevice_h
#define __mitkToFCameraPMDMITKPlayerDevice_h

#include <mitkToFHardwareExports.h>
#include <mitkCommon.h>
#include <mitkToFCameraDevice.h>
#include <mitkToFCameraPMDDevice.h>

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkMultiThreader.h>
#include <itkFastMutexLock.h>


namespace mitk
{
  /**
  * @brief Device class representing a player for MITK-ToF raw images.
  *
  * @ingroup ToFHardware
  */
  class MITK_TOFHARDWARE_EXPORT ToFCameraPMDMITKPlayerDevice : public ToFCameraPMDDevice
  {
  public:

    mitkClassMacro( ToFCameraPMDMITKPlayerDevice , ToFCameraPMDDevice );

    itkNewMacro( Self );

  protected:

    ToFCameraPMDMITKPlayerDevice();

    ~ToFCameraPMDMITKPlayerDevice();

    std::string m_InputFileName; ///< file name of input data

  private:

  };
} //END mitk namespace
#endif
