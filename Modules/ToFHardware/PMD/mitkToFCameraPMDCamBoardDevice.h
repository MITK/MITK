/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkToFCameraPMDCamBoardDevice_h
#define mitkToFCameraPMDCamBoardDevice_h

#include <MitkPMDExports.h>
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
  class MITKPMD_EXPORT ToFCameraPMDCamBoardDevice : public ToFCameraPMDDevice
  {
  public:

    mitkClassMacro( ToFCameraPMDCamBoardDevice , ToFCameraPMDDevice );

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /*!
    \brief set a BaseProperty
    */
    virtual void SetProperty( const char *propertyKey, BaseProperty* propertyValue );
    /*!
    \brief Setting the region of interest, the camera is configured to only output a certain area of the image.
    \param leftUpperCornerX x value of left upper corner of region
    \param leftUpperCornerY y value of left upper corner of region
    \param width width of region
    \param height height of region
    */
    void SetRegionOfInterest( unsigned int leftUpperCornerX, unsigned int leftUpperCornerY, unsigned int width, unsigned int height );

    //void GetAllImages(float* distanceArray, float* amplitudeArray, float* intensityArray, char* sourceDataArray, int requiredImageSequence, int& capturedImageSequence, unsigned char* rgbDataArray);

  protected:

    ToFCameraPMDCamBoardDevice();

    ~ToFCameraPMDCamBoardDevice();

  private:

  };
} //END mitk namespace
#endif
