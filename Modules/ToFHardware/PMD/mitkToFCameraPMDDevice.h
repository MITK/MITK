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
#ifndef __mitkToFCameraPMDDevice_h
#define __mitkToFCameraPMDDevice_h

#include "mitkPMDModuleExports.h"
#include "mitkCommon.h"
#include "mitkToFCameraDevice.h"
#include "mitkToFCameraPMDController.h"

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkMultiThreader.h"
#include "itkFastMutexLock.h"


namespace mitk
{
  /**
  * @brief Interface for all representations of PMD ToF devices.
  * ToFCameraPMDDevice internally holds an instance of ToFCameraPMDController and starts a thread
  * that continuously grabs images from the controller. A buffer structure buffers the last acquired images
  * to provide the image data loss-less.
  *
  *\throws mitkException In case of no connection, an exception is thrown!
  *
  * @ingroup ToFHardware
  */
  class MITK_PMDMODULE_EXPORT ToFCameraPMDDevice : public ToFCameraDevice
  {
  public:

    mitkClassMacro( ToFCameraPMDDevice , ToFCameraDevice );

    itkNewMacro( Self );

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
    \brief updated the controller hold by this device
    \throws mitkException In case of no connection, an exception is thrown!
    */
    virtual void UpdateCamera();
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
//    TODO: Buffer size currently set to 1. Once Buffer handling is working correctly, method may be reactivated
//    /* // * TODO: Reenable doxygen comment when uncommenting, disabled to fix doxygen warning see bug 12882
//    \brief pure virtual method resetting the buffer using the specified bufferSize. Has to be implemented by sub-classes
//    \param bufferSize buffer size the buffer should be reset to
//    */
//    virtual void ResetBuffer(int bufferSize) = 0;
    //TODO add/correct documentation for requiredImageSequence and capturedImageSequence in the GetAllImages, GetDistances, GetIntensities and GetAmplitudes methods.

    /*!
    \brief returns the corresponding camera controller
    */
    ToFCameraPMDController::Pointer GetController();

    /*!
    \brief set a BaseProperty
    */
    virtual void SetProperty( const char *propertyKey, BaseProperty* propertyValue );

  protected:

    ToFCameraPMDDevice();

    ~ToFCameraPMDDevice();

    /*!
    \brief method for allocating m_SourceDataArray and m_SourceDataBuffer
    */
    virtual void AllocateSourceData();
    /*!
    \brief method for cleaning up memory allocated for m_SourceDataArray and m_SourceDataBuffer
    */
    virtual void CleanUpSourceData();

    /*!
    \brief Thread method continuously acquiring images from the ToF hardware
    */
    static ITK_THREAD_RETURN_TYPE Acquire(void* pInfoStruct);
    /*!
    \brief moves the position pointer m_CurrentPos to the next position in the buffer if that's not the next free position to prevent reading from an empty buffer
    */
    void GetNextPos();

    ToFCameraPMDController::Pointer m_Controller; ///< corresponding CameraController
    char** m_SourceDataBuffer; ///< buffer holding the last acquired images
    char* m_SourceDataArray; ///< array holding the current PMD source data

  private:

  };
} //END mitk namespace
#endif
