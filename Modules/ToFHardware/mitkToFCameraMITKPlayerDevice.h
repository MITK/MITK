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
#ifndef __mitkToFCameraMITKPlayerDevice_h
#define __mitkToFCameraMITKPlayerDevice_h

#include "mitkToFHardwareExports.h"
#include "mitkCommon.h"
#include "mitkToFCameraDevice.h"
#include "mitkToFCameraMITKPlayerController.h"

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkMultiThreader.h"
#include "itkFastMutexLock.h"


namespace mitk
{
  /**
  * @brief Device class representing a player for MITK-ToF images.
  *
  * @ingroup ToFHardware
  */
  class MITK_TOFHARDWARE_EXPORT ToFCameraMITKPlayerDevice : public ToFCameraDevice
  {
  public:

    mitkClassMacro( ToFCameraMITKPlayerDevice , ToFCameraDevice );

    itkNewMacro( Self );

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
    \brief gets the rgb data from the ToF camera. Caution! The user is responsible for allocating and deleting the images.
    \param rgbArray contains the returned rgb data as an array.
    \param imageSequence the actually captured image sequence number
    */
    virtual void GetRgb(unsigned char* rgbArray, int& imageSequence);
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
    \brief Set file name where the data is recorded
    \param inputFileName name of input file which should be played
    */
    virtual void SetInputFileName(std::string inputFileName);

    /*!
    \brief set a BaseProperty
    */
    virtual void SetProperty( const char *propertyKey, BaseProperty* propertyValue );

  protected:

    ToFCameraMITKPlayerDevice();

    ~ToFCameraMITKPlayerDevice();
    /*!
    \brief updates the camera for image acquisition
    */
    virtual void UpdateCamera();
    /*!
    \brief Thread method continuously acquiring images from the specified input file
    */
    static ITK_THREAD_RETURN_TYPE Acquire(void* pInfoStruct);
    /*!
    \brief Clean up memory (pixel buffers)
    */
    void CleanUpDataBuffers();
    /*!
    \brief Allocate pixel buffers
    */
    void AllocateDataBuffers();

    ToFCameraMITKPlayerController::Pointer m_Controller; ///< member holding the corresponding controller
    std::string m_InputFileName; ///< member holding the file name of the current input file

  private:

    float** m_DistanceDataBuffer; ///< buffer holding the last distance images
    float** m_AmplitudeDataBuffer; ///< buffer holding the last amplitude images
    float** m_IntensityDataBuffer; ///< buffer holding the last intensity images
    unsigned char** m_RGBDataBuffer; ///< buffer holding the last rgb images

  };
} //END mitk namespace
#endif
