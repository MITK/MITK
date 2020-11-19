/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __mitkKinectController_h
#define __mitkKinectController_h

#include <MitkKinectExports.h>
#include "mitkCommon.h"
#include "mitkToFConfig.h"

#include "itkObject.h"
#include "itkObjectFactory.h"

namespace mitk
{
  /**
  * @brief Interface to the Kinect camera
  *
  *
  * @ingroup ToFHardware
  */
  class MITKKINECT_EXPORT KinectController : public itk::Object
  {
  public:

    mitkClassMacroItkParent( KinectController , itk::Object );

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    unsigned int GetCaptureWidth() const;
    unsigned int GetCaptureHeight() const;
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
    \param amplitudes
    \param rgb pointer to memory location where rgb information should be stored
    */
    void GetAllData(float* distances, float* amplitudes, unsigned char* rgb);

  protected:

    KinectController();

    ~KinectController();

  private:
    class KinectControllerPrivate;
    KinectControllerPrivate *d;

  };
} //END mitk namespace
#endif
