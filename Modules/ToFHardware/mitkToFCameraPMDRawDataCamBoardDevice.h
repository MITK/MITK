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
#ifndef __mitkToFCameraPMDRawDataCamBoardDevice_h
#define __mitkToFCameraPMDRawDataCamBoardDevice_h

#include "mitkToFHardwareExports.h"
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
  class MITK_TOFHARDWARE_EXPORT ToFCameraPMDRawDataCamBoardDevice : public ToFCameraPMDRawDataDevice
  {
  public: 

    mitkClassMacro( ToFCameraPMDRawDataCamBoardDevice , ToFCameraPMDRawDataDevice );

    itkNewMacro( Self );
    
    virtual void SetProperty( const char *propertyKey, BaseProperty* propertyValue );

    virtual void GetChannelSourceData(short* sourceData, vtkShortArray* vtkChannelArray );


  protected:

    ToFCameraPMDRawDataCamBoardDevice();

    ~ToFCameraPMDRawDataCamBoardDevice();
  private:

  };
} //END mitk namespace
#endif // __mitkToFCameraPMDRawDataCamBoardDevice_h
