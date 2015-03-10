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
#ifndef __mitkToFCameraMESADevice_h
#define __mitkToFCameraMESADevice_h

#include <MitkMESASR4000ModuleExports.h>
#include "mitkCommon.h"
#include "mitkToFCameraDevice.h"
#include "mitkToFCameraMESAController.h"

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkMultiThreader.h"
#include "itkFastMutexLock.h"


namespace mitk
{
  /**
  * @brief Interface for all representations of MESA ToF devices.
  * ToFCameraMESADevice internally holds an instance of ToFCameraMESAController and starts a thread
  * that continuously grabs images from the controller. A buffer structure buffers the last acquired images
  * to provide the image data loss-less.
  *
  * @ingroup ToFHardware
  */
  class MITKMESASR4000_EXPORT ToFCameraMESADevice : public ToFCameraDevice
  {
  public:

    mitkClassMacro( ToFCameraMESADevice , ToFCameraDevice );

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /*!
    \brief opens a connection to the ToF camera
    */
    virtual bool OnConnectCamera();
    /*!
    \brief closes the connection to the camera
    */
    virtual bool DisconnectCamera();
    /*!
    \brief starts the continuous updating of the camera.
    A separate thread updates the source data, the main thread processes the source data and creates images and coordinates
    */
    virtual void StartCamera();
    /*!
    \brief stops the continuous updating of the camera
    */
    virtual void StopCamera();
    /*!
    \brief updates the camera for image acquisition
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
    \brief gets the intensity data from the ToF camera as a greyscale image. Caution! The user is responsible for allocating and deleting the images.
    \param intensityArray contains the returned intensities data as an array.
    \param imageSequence the actually captured image sequence number
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
    \param distanceArray contains the returned distance data as an array.
    \param amplitudeArray contains the returned amplitude data as an array.
    \param intensityArray contains the returned intensity data as an array.
    \param sourceDataArray contains the complete source data from the camera device.
    \param requiredImageSequence the required image sequence number
    \param capturedImageSequence the actually captured image sequence number
    */
    virtual void GetAllImages(float* distanceArray, float* amplitudeArray, float* intensityArray, char* sourceDataArray,
                              int requiredImageSequence, int& capturedImageSequence, unsigned char* rgbDataArray=NULL);

    /*!
    \brief returns the corresponding camera controller
    */
    ToFCameraMESAController::Pointer GetController();

    /*!
    \brief set a BaseProperty
    */
    virtual void SetProperty( const char *propertyKey, BaseProperty* propertyValue );

  protected:

    ToFCameraMESADevice();

    ~ToFCameraMESADevice();

    /*!
    \brief Thread method continuously acquiring images from the ToF hardware
    */
    static ITK_THREAD_RETURN_TYPE Acquire(void* pInfoStruct);
    /*!
    \brief moves the position pointer m_CurrentPos to the next position in the buffer if that's not the next free position to prevent reading from an empty buffer
    */
    void GetNextPos();

    ToFCameraMESAController::Pointer m_Controller; ///< corresponding CameraController

    float** m_DistanceDataBuffer; ///< buffer holding the last distance images
    float** m_AmplitudeDataBuffer; ///< buffer holding the last amplitude images
    float** m_IntensityDataBuffer; ///< buffer holding the last intensity images

  private:

  };
} //END mitk namespace
#endif
