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
#ifndef __mitkToFCameraPMDCamBoardDevice_h
#define __mitkToFCameraPMDCamBoardDevice_h

#include "mitkPMDModuleExports.h"
#include "mitkCommon.h"
#include "mitkToFCameraDevice.h"
#include "mitkToFCameraPMDDevice.h"
#include "mitkToFCameraPMDCamBoardController.h"

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkMultiThreader.h"
#include "itkFastMutexLock.h"


namespace mitk
{
  /**
  * @brief Device class representing a PMD CamBoard camera
  *
  * @ingroup ToFHardware
  */
  class MITK_PMDMODULE_EXPORT ToFCameraPMDCamBoardDevice : public ToFCameraPMDDevice
  {
  public:

    mitkClassMacro( ToFCameraPMDCamBoardDevice , ToFCameraPMDDevice );

    itkNewMacro( Self );

    /*!
    \brief set a BaseProperty
    */
    virtual void SetProperty( const char *propertyKey, BaseProperty* propertyValue );
    /*!
    \brief Setting the region of interest, the camera is configured to only output a certain area of the image.
    \param leftUpperCornerX x value of left upper corner of region
    \param leftUpperCornerX y value of left upper corner of region
    \param width width of region
    \param height height of region
    */
    void SetRegionOfInterest( unsigned int leftUpperCornerX, unsigned int leftUpperCornerY, unsigned int width, unsigned int height );

    void GetAllImages(float* distanceArray, float* amplitudeArray, float* intensityArray, char* sourceDataArray, int requiredImageSequence, int& capturedImageSequence, unsigned char* rgbDataArray);

  protected:

    ToFCameraPMDCamBoardDevice();

    ~ToFCameraPMDCamBoardDevice();

  private:

  };
} //END mitk namespace
#endif
