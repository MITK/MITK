/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __mitkToFCameraPMDCamCubeDevice_h
#define __mitkToFCameraPMDCamCubeDevice_h

#include <MitkPMDExports.h>
#include "mitkCommon.h"
#include "mitkToFCameraDevice.h"
#include "mitkToFCameraPMDDevice.h"
#include "mitkToFCameraPMDCamCubeController.h"

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkMultiThreader.h"
#include "itkFastMutexLock.h"


namespace mitk
{
  /**
  * @brief Device class representing a PMD CamCube camera
  *
  *
  * @ingroup ToFHardware
  */
  class MITKPMD_EXPORT ToFCameraPMDCamCubeDevice : public ToFCameraPMDDevice
  {
  public:

    mitkClassMacro( ToFCameraPMDCamCubeDevice , ToFCameraPMDDevice );

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    virtual void SetProperty( const char *propertyKey, BaseProperty* propertyValue );

  protected:

    ToFCameraPMDCamCubeDevice();

    ~ToFCameraPMDCamCubeDevice();

  private:

  };
} //END mitk namespace
#endif
