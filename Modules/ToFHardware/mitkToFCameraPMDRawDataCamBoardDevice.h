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

    bool ConnectCamera();
    void GetIntensities(float* intensityArray, int& imageSequence);
    void GetAmplitudes(float* amplitudeArray, int& imageSequence);
    void GetDistances(float* distanceArray, int& imageSequence);
    void GetAllImages(float* distanceArray, float* amplitudeArray, float* intensityArray, char* sourceDataArray, int requiredImageSequence, int& capturedImageSequence, unsigned char* rgbDataArray);

  protected:

    ToFCameraPMDRawDataCamBoardDevice();

    ~ToFCameraPMDRawDataCamBoardDevice();
  private:
    void ResizeOutputImage(float* in, float* out);
    // member variables
    //unsigned int m_OriginControllerHeight;
    //unsigned int m_OriginControllerWidth;
  };
} //END mitk namespace
#endif // __mitkToFCameraPMDRawDataCamBoardDevice_h
