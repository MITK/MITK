/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __mitkToFCameraPMDRawDataCamCubeDevice_h
#define __mitkToFCameraPMDRawDataCamCubeDevice_h

#include <MitkPMDExports.h>
#include "mitkCommon.h"
#include "mitkToFCameraDevice.h"
#include "mitkToFCameraPMDRawDataDevice.h"

namespace mitk
{
  /**
  * @brief Device class representing a PMD CamCube camera
  *
  *
  * @ingroup ToFHardwareMBI
  */
  class MITKPMD_EXPORT ToFCameraPMDRawDataCamCubeDevice : public ToFCameraPMDRawDataDevice
  {
  public:

    mitkClassMacro( ToFCameraPMDRawDataCamCubeDevice , ToFCameraPMDRawDataDevice );

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    virtual void SetProperty( const char *propertyKey, BaseProperty* propertyValue );

    virtual void GetChannelSourceData(short* sourceData, vtkShortArray* vtkChannelArray );


  protected:

    ToFCameraPMDRawDataCamCubeDevice();

    ~ToFCameraPMDRawDataCamCubeDevice();
  private:

  };
} //END mitk namespace
#endif
