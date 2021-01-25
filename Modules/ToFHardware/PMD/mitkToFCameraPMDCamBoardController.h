/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __mitkToFCameraPMDCamBoardController_h
#define __mitkToFCameraPMDCamBoardController_h

#include "MitkPMDExports.h"
#include "mitkCommon.h"
#include "mitkToFCameraPMDController.h"

#include "itkObject.h"
#include "itkObjectFactory.h"

namespace mitk
{
  /**
  * @brief Interface to the Time-of-Flight (ToF) camera PMD CamBoard
  *
  *
  * @ingroup ToFHardware
  */
  class MITKPMD_EXPORT ToFCameraPMDCamBoardController : public ToFCameraPMDController
  {
  public:

    mitkClassMacro( ToFCameraPMDCamBoardController , ToFCameraPMDController );

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    /*!
    \brief opens a connection to the ToF camera and initializes the hardware specific members
    \return returns whether the connection was successful (true) or not (false)
    */
    virtual bool OpenCameraConnection();
    /*!
    \brief sets an additional distance offset which will be added to all distance values.
    \param offset offset in m
    */
    bool SetDistanceOffset( float offset );
    /*!
    \brief returns the currently applied distance offset in m
    */
    float GetDistanceOffset();
    /*!
    \brief Setting the region of interest, the camera is configured to only output a certain area of the image.
    \param leftUpperCornerX x value of left upper corner of region
    \param leftUpperCornerY y value of left upper corner of region
    \param width width of region
    \param height height of region
    */
    bool SetRegionOfInterest( unsigned int leftUpperCornerX, unsigned int leftUpperCornerY, unsigned int width, unsigned int height );
    /*!
    \brief Setting the region of interest, the camera is configured to only output a certain area of the image.
    \param roi region of interest. roi[0]: x value of left upper corner, roi[1]: y value of left upper corner, roi[2]: width, roi[3]: height
    */
    bool SetRegionOfInterest( unsigned int roi[4] );
    /*!
    \brief returns the region of interest currently set
    \return currently set region of interest.
    */
    unsigned int* GetRegionOfInterest();
    /*!
    \brief Sets the field of view of the camera lens.
    \param fov field of view in degrees. The default value is 40 degrees.
    */
    bool SetFieldOfView( float fov );

  protected:

    ToFCameraPMDCamBoardController();

    ~ToFCameraPMDCamBoardController();

    /*
    \brief Transform the output of the camera, i.e. cut invalid pixels, and rotate 90 degrees
           counterclockwise
    \param input data array of original size (207x204)
    \param rotated output data array of reduced size (200x200)
    */
    virtual void TransformCameraOutput(float* in, float* out, bool isDist);

  };
} //END mitk namespace
#endif
