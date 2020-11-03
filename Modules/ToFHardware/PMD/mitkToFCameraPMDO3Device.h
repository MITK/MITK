/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __mitkToFCameraPMDO3Device_h
#define __mitkToFCameraPMDO3Device_h

#include <MitkPMDExports.h>
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
  class MITKPMD_EXPORT ToFCameraPMDO3Device : public ToFCameraPMDDevice
  {
  public:

    mitkClassMacro( ToFCameraPMDO3Device , ToFCameraPMDDevice );

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

  protected:

    ToFCameraPMDO3Device();

    ~ToFCameraPMDO3Device();

  private:

  };
} //END mitk namespace
#endif
