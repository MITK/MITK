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
#ifndef __mitkToFCameraPMDO3Device_h
#define __mitkToFCameraPMDO3Device_h

#include "mitkPMDModuleExports.h"
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
  * @brief Device class representing a PMD O3 camera
  *
  *
  * @ingroup ToFHardware
  */
  class MITK_PMDMODULE_EXPORT ToFCameraPMDO3Device : public ToFCameraPMDDevice
  {
  public:

    mitkClassMacro( ToFCameraPMDO3Device , ToFCameraPMDDevice );

    itkNewMacro( Self );

  protected:

    ToFCameraPMDO3Device();

    ~ToFCameraPMDO3Device();

  private:

  };
} //END mitk namespace
#endif
