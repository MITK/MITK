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
#ifndef __mitkToFCameraPMDCamCubeController_h
#define __mitkToFCameraPMDCamCubeController_h

#include "mitkPMDModuleExports.h"
#include "mitkCommon.h"
#include "mitkToFCameraPMDController.h"

#include "itkObject.h"
#include "itkObjectFactory.h"

namespace mitk
{
  /**
  * @brief Interface to the Time-of-Flight (ToF) camera PMD CamCube
  *
  *
  * @ingroup ToFHardware
  */
  class MITK_PMDMODULE_EXPORT ToFCameraPMDCamCubeController : public ToFCameraPMDController
  {
  public:

    mitkClassMacro( ToFCameraPMDCamCubeController , ToFCameraPMDController );

    itkNewMacro( Self );

    /*!
    \brief opens a connection to the ToF camera and initializes the hardware specific members
    \return returns whether the connection was successful (true) or not (false)
    */
    virtual bool OpenCameraConnection();
    /*!
    \brief sets an additional distance offset which will be added to all distance values.
    \param offset offset in m
    \return returns whether set operation was successful (true) or not (false)
    */
    bool SetDistanceOffset( float offset );
    /*!
    \brief returns the currently applied distance offset in m
    \param offset offset in m
    */
    float GetDistanceOffset();
    /*!
    \brief Setting the region of interest, the camera is configured to only output a certain area of the image.
    \param leftUpperCornerX x value of left upper corner of region
    \param leftUpperCornerX y value of left upper corner of region
    \param width width of region
    \param height height of region
    \return returns whether set operation was successful (true) or not (false)
    */
    bool SetRegionOfInterest( unsigned int leftUpperCornerX, unsigned int leftUpperCornerY, unsigned int width, unsigned int height );
    /*!
    \brief Setting the region of interest, the camera is configured to only output a certain area of the image.
    \param roi region of interest. roi[0]: x value of left upper corner, roi[1]: y value of left upper corner, roi[2]: width, roi[3]: height
    \return returns whether set operation was successful (true) or not (false)
    */
    bool SetRegionOfInterest( unsigned int roi[4] );
    /*!
    \brief returns the region of interest currently set
    \return currently set region of interest.
    */
    unsigned int* GetRegionOfInterest();
    /*!
    \brief sets the exposure mode of the CamCube
    \param mode exposure mode. 0: normal mode (one exposure), 1: Suppression of motion blur (SMB), minimizes the time needed to capture
    a distance image from the camera which results in a reduced amount of motion artifact but may lead to increased noise.
    \return returns whether set operation was successful (true) or not (false)
    */
    bool SetExposureMode( int mode );
    /*!
    \brief Sets the field of view of the camera lens.
    \param fov field of view in degrees. The default value is 40 degrees.
    \return returns whether set operation was successful (true) or not (false)
    */
    bool SetFieldOfView( float fov );
    /*!
    \brief Enable/Disable PMD fixed pattern noise (FPN) calibration
    \param on enabled (true), disabled (false)
    \return returns whether set operation was successful (true) or not (false)
    */
    bool SetFPNCalibration( bool on );
    /*!
    \brief Enable/Disable PMD fixed pattern phase noise (FPPN) calibration
    \param on enabled (true), disabled (false)
    \return returns whether set operation was successful (true) or not (false)
    */
    bool SetFPPNCalibration( bool on );
    /*!
    \brief Enable/Disable PMD linearity calibration
    \param on enabled (true), disabled (false)
    \return returns whether set operation was successful (true) or not (false)
    */
    bool SetLinearityCalibration( bool on );
    /*!
    \brief Enable/Disable PMD lens calibration
    \param on enabled (true), disabled (false)
    \return returns whether set operation was successful (true) or not (false)
    */
    bool SetLensCalibration( bool on );

  protected:

    ToFCameraPMDCamCubeController();

    ~ToFCameraPMDCamCubeController();

  private:

  };
} //END mitk namespace
#endif
