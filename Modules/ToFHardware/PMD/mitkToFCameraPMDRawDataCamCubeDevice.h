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

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual void SetProperty( const char *propertyKey, BaseProperty* propertyValue );

    virtual void GetChannelSourceData(short* sourceData, vtkShortArray* vtkChannelArray );


  protected:

    ToFCameraPMDRawDataCamCubeDevice();

    ~ToFCameraPMDRawDataCamCubeDevice();
  private:

  };
} //END mitk namespace
#endif
