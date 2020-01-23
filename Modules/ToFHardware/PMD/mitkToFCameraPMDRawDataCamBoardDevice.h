/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __mitkToFCameraPMDRawDataCamBoardDevice_h
#define __mitkToFCameraPMDRawDataCamBoardDevice_h

#include <MitkPMDExports.h>
#include "mitkCommon.h"
#include "mitkToFCameraDevice.h"
#include "mitkToFCameraPMDRawDataDevice.h"

namespace mitk
{
  /**
  * @brief Device class representing a PMD CamBoard camera
  *
  *
  * @ingroup ToFHardwareMBI
  */
  class MITKPMD_EXPORT ToFCameraPMDRawDataCamBoardDevice : public ToFCameraPMDRawDataDevice
  {
  public:

    mitkClassMacro( ToFCameraPMDRawDataCamBoardDevice , ToFCameraPMDRawDataDevice );

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self)
    /*!
    \brief set a BaseProperty
    */
    virtual void SetProperty( const char *propertyKey, BaseProperty* propertyValue );
    /*!
    \brief Transforms the sourceData into an array with four tuples holding the channels for
           raw data reconstruction.
    */
    virtual void GetChannelSourceData(short* sourceData, vtkShortArray* vtkChannelArray );
    /*!
    \brief Establishes camera connection and sets the class variables
    */
    bool ConnectCamera();
    /*!
    \brief Returns intensity data
    */
    void GetIntensities(float* intensityArray, int& imageSequence);
    /*!
    \brief Returns amplitude data
    */
    void GetAmplitudes(float* amplitudeArray, int& imageSequence);
    /*!
    \brief Returns distance data
    */
    void GetDistances(float* distanceArray, int& imageSequence);
    /*!
    \brief Returns all image data at once.
    */
    void GetAllImages(float* distanceArray, float* amplitudeArray, float* intensityArray, char* sourceDataArray, int requiredImageSequence, int& capturedImageSequence, unsigned char* rgbDataArray=nullptr);

  protected:

    ToFCameraPMDRawDataCamBoardDevice();

    ~ToFCameraPMDRawDataCamBoardDevice();
  private:
    /*!
    \brief Method performs resizing of the image data and flips it upside down
    */
    void ResizeOutputImage(float* in, float* out);
  };
} //END mitk namespace
#endif // __mitkToFCameraPMDRawDataCamBoardDevice_h
