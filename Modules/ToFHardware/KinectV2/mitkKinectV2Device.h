/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef __mitkKinectV2Device_h
#define __mitkKinectV2Device_h

#include <mitkCommon.h>
#include <MitkKinectV2Exports.h>
#include "mitkToFCameraDevice.h"
#include "mitkKinectV2Controller.h"

#include <itkObject.h>
#include <itkObjectFactory.h>
#include <itkMultiThreader.h>
#include <itkFastMutexLock.h>

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>


namespace mitk
{
  /**
  * @brief Interface for all representations of Microsoft Kinect V2 devices.
  * Kinect2Device internally holds an instance of Kinect2Controller and starts a thread
  * that continuously grabs images from the controller. A buffer structure buffers the last acquired images
  * to provide the image data loss-less.
  *
  * \throws mitkException In case of no connection, an exception is thrown!
  *
  * @ingroup ToFHardware
  */
  class MITKKINECTV2_EXPORT KinectV2Device : public ToFCameraDevice
  {
  public:

    mitkClassMacro( KinectV2Device , ToFCameraDevice );
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self)
    /** Prints the framerate to the console every 100 frames.
     *  Mainly for debugging, deactivated by default.
     *  Refers to a static variable, means it is acivated/deactivated
     *  for all instances.
     */
    itkSetMacro(PrintFrameRate, bool);

    /*!
    \brief opens a connection to the ToF camera
    \throws mitkException In case of no connection, an exception is thrown!
    */
    virtual bool OnConnectCamera();
    /*!
    \brief closes the connection to the camera
    */
    virtual bool DisconnectCamera();
    /*!
    \brief starts the continuous updating of the camera.
    A separate thread updates the source data, the main thread processes the source data and creates images and coordinates
    \throws mitkException In case of no connection, an exception is thrown!
    */
    virtual void StartCamera();
    /*!
    \brief stops the continuous updating of the camera
    */
    virtual void StopCamera();
    /*!
    \brief updates the camera for image acquisition
    \throws mitkException In case of no connection, an exception is thrown!
    */
    virtual void UpdateCamera();
    /*!
    \brief returns whether the camera is currently active or not
    */
    virtual bool IsCameraActive();
    /*!
    \brief gets the amplitude data from the ToF camera as the strength of the active illumination of every pixel. Caution! The user is responsible for allocating and deleting the images.
    These values can be used to determine the quality of the distance values. The higher the amplitude value, the higher the accuracy of the according distance value
    \param imageSequence the actually captured image sequence number
    \param amplitudeArray contains the returned amplitude data as an array.
    */
    virtual void GetAmplitudes(float* amplitudeArray, int& imageSequence);
    /*!
    \brief Does nothing for Kinect V2 as there is no intensity data provided by the device.
    *
    * The method is an empty implementation, because the interface (ToFCameraDevice) requires it.
    */
    virtual void GetIntensities(float* intensityArray, int& imageSequence);
    /*!
    \brief gets the distance data from the ToF camera measuring the distance between the camera and the different object points in millimeters. Caution! The user is responsible for allocating and deleting the images.
    \param distanceArray contains the returned distances data as an array.
    \param imageSequence the actually captured image sequence number
    */
    virtual void GetDistances(float* distanceArray, int& imageSequence);
    /*!
    \brief gets the 3 images (distance, amplitude, intensity) from the ToF camera. Caution! The user is responsible for allocating and deleting the images.
    \param distanceArray Contains the distance data as an array.
    \param amplitudeArray Contains the infrared image.
    \param intensityArray Does nothing for Kinect V2.
    \param sourceDataArray Does nothing for Kinect V2.
    \param requiredImageSequence The required image sequence number.
    \param capturedImageSequence Does nothing for Kinect V2.
    \param rgbDataArray
    */
    virtual void GetAllImages(float* distanceArray, float* amplitudeArray, float* intensityArray, char* sourceDataArray,
      int requiredImageSequence, int& capturedImageSequence, unsigned char* rgbDataArray=nullptr);
    /*!
    \brief returns the corresponding camera controller
    */
    KinectV2Controller::Pointer GetController();

    /*!
    \brief returns the width of the RGB image
    */
    int GetRGBCaptureWidth();
    /*!
    \brief returns the height of the RGB image
    */
    int GetRGBCaptureHeight();

  protected:

    KinectV2Device();

    ~KinectV2Device();

    /*!
    \brief Thread method continuously acquiring images from the ToF hardware
    */
    static ITK_THREAD_RETURN_TYPE Acquire(void* pInfoStruct);
    static bool m_PrintFrameRate; ///< prints the framerate to the console every 100 frames, deactivated by default

    KinectV2Controller::Pointer m_Controller; ///< corresponding CameraController

    float** m_DistanceDataBuffer; ///< buffer holding the last distance images
    float** m_AmplitudeDataBuffer; ///< buffer holding the last amplitude images
    unsigned char** m_RGBDataBuffer; ///< buffer holding the last RGB image



    size_t m_DepthBufferSize; ///< Size of depth buffer (i.e. memory size of depth and infrared image)
    size_t m_RGBBufferSize; ///< Size of RGB buffer (i.e. memory size of RGB image)

    vtkSmartPointer<vtkPolyData> m_PolyData; ///< Surface generated via the Kinect V2 SDK with default/unknown calibration.
  };
} //END mitk namespace
#endif
