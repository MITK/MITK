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
#ifndef __mitkToFCameraPMDCamCubeDevice_h
#define __mitkToFCameraPMDCamCubeDevice_h

#include "mitkPMDModuleExports.h"
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
  class MITK_PMDMODULE_EXPORT ToFCameraPMDCamCubeDevice : public ToFCameraPMDDevice
  {
  public:

    mitkClassMacro( ToFCameraPMDCamCubeDevice , ToFCameraPMDDevice );

    itkNewMacro( Self );

    virtual void SetProperty( const char *propertyKey, BaseProperty* propertyValue );

  protected:

    ToFCameraPMDCamCubeDevice();

    ~ToFCameraPMDCamCubeDevice();

  private:

  };
} //END mitk namespace
#endif
