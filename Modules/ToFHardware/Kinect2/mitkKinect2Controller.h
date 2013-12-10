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
#ifndef __mitkKinect2Controller_h
#define __mitkKinect2Controller_h

#include "mitkKinect2ModuleExports.h"
#include "mitkCommon.h"
#include "mitkToFConfig.h"

#include "itkObject.h"
#include "itkObjectFactory.h"

namespace mitk
{
  /**
  * @brief Interface to the Kinect 2 camera
  *
  *
  * @ingroup ToFHardware
  */
  class MITK_KINECT2MODULE_EXPORT Kinect2Controller : public itk::Object
  {
  public:

    mitkClassMacro( Kinect2Controller , itk::Object );

    itkNewMacro( Self );
    int GetRGBCaptureWidth() const;
    int GetRGBCaptureHeight() const;
    int GetDepthCaptureWidth() const;
    int GetDepthCaptureHeight() const;

    bool GetUseIR() const;

    void SetUseIR(bool useIR);

    /*!
    \brief opens a connection to the Kinect camera.
    \throws mitkException In case of no connection, an exception is thrown!
    */
    virtual bool OpenCameraConnection();
    /*!
    \brief closes the connection to the camera
    */
    virtual bool CloseCameraConnection();
    /*!
    \brief updates the camera. The update function of the hardware interface is called only when new data is available
    \throws mitkException In case of no connection, an exception is thrown!
    */
    virtual bool UpdateCamera();
    /*!
    \brief acquire new distance data from the Kinect camera
    \param distances pointer to memory location where distances should be stored
    */
    void GetDistances(float* distances);
    void GetAmplitudes(float* amplitudes);
    void GetIntensities(float* intensities);
    /*!
    \brief acquire new rgb data from the Kinect camera
    \param rgb pointer to memory location where rgb information should be stored
    */
    void GetRgb(unsigned char* rgb);
    /*!
    \brief convenience method for faster access to distance and rgb data
    \param distances pointer to memory location where distances should be stored
    \param rgb pointer to memory location where rgb information should be stored
    */
    void GetAllData(float* distances, float* amplitudes, unsigned char* rgb);

  protected:

    Kinect2Controller();

    ~Kinect2Controller();

  private:
    class Kinect2ControllerPrivate;
    Kinect2ControllerPrivate *d;

  };
} //END mitk namespace
#endif
