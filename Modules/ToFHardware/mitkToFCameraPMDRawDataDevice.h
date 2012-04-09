/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2010-05-27 16:06:53 +0200 (Do, 27 Mai 2010) $
Version:   $Revision:  $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __mitkToFCameraPMDRawDataDevice_h
#define __mitkToFCameraPMDRawDataDevice_h

#include "mitkToFHardwareExports.h"
#include "mitkToFCameraDevice.h"
#include "mitkToFCameraPMDController.h"
#include "mitkThreadedToFRawDataReconstruction.h"

namespace mitk
{
  /**
  * @brief Interface for all representations of PMD ToF devices. 
  * ToFCameraPMDDevice internally holds an instance of ToFCameraPMDController and starts a thread 
  * that continuously grabs images from the controller. A buffer structure buffers the last acquired images
  * to provide the image data loss-less.
  *
  * @ingroup ToFHardware
  */
  class MITK_TOFHARDWARE_EXPORT ToFCameraPMDRawDataDevice : public ToFCameraDevice
  {
  public: 

    mitkClassMacro( ToFCameraPMDRawDataDevice , ToFCameraDevice );

    itkNewMacro( Self );

    itkSetMacro(ChannelSize, int);
    itkGetMacro(ChannelSize, int);
    /*!
    \brief opens a connection to the ToF camera
    */
    virtual bool ConnectCamera();
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
                              int requiredImageSequence, int& capturedImageSequence);
//    TODO: Buffer size currently set to 1. Once Buffer handling is working correctly, method may be reactivated
//    /*!
//    \brief pure virtual method resetting the buffer using the specified bufferSize. Has to be implemented by sub-classes
//    \param bufferSize buffer size the buffer should be reset to
//    */
//    virtual void ResetBuffer(int bufferSize) = 0;
    //TODO add/correct documentation for requiredImageSequence and capturedImageSequence in the GetAllImages, GetDistances, GetIntensities and GetAmplitudes methods.

    /*!
    \brief returns the corresponding camera controller
    */
    ToFCameraPMDController::Pointer GetController();

    virtual void GetChannelSourceData(short* /*sourceData*/, vtkShortArray* /*vtkChannelArray*/ ){}; 

    /*!
    \brief set a BaseProperty
    */
    virtual void SetProperty( const char *propertyKey, BaseProperty* propertyValue );

  protected:

    ToFCameraPMDRawDataDevice();

    ~ToFCameraPMDRawDataDevice();

    /*!
    \brief method for allocating m_SourceDataArray and m_SourceDataBuffer
    */
    virtual void AllocateSourceData();

    /*!
    \brief method for cleaning up memory allocated for m_SourceDataArray and m_SourceDataBuffer
    */
    virtual void CleanUpSourceData();

    /*!
    \brief method for allocating memory for pixel arrays m_IntensityArray, m_DistanceArray and m_AmplitudeArray
    */
    virtual void AllocatePixelArrays();
    /*!
    \brief method for cleanup memory allocated for pixel arrays m_IntensityArray, m_DistanceArray and m_AmplitudeArray
    */
    virtual void CleanupPixelArrays();
    /*!
    \brief Thread method continuously acquiring images from the ToF hardware
    */
    static ITK_THREAD_RETURN_TYPE Acquire(void* pInfoStruct);

    /*!
    \brief moves the position pointer m_CurrentPos to the next position in the buffer if that's not the next free position to prevent reading from an empty buffer
    */
    void GetNextPos();

    /*!
    \brief gets the image data and flips it according to user needs Caution! The user is responsible for allocating and deleting the data.
    \param imageData contains array to the input data.
    \param flippedData contains flipped output array - Caution! The user is responsible for allocating and deleting the data. Size should be equal to imageData!
    \param xAxis flag is set to flip around x axis (1 - set, 0 - not set).
    \param yAxis flag is set to flip around y axis (1 - set, 0 - not set).
    \param dimension contains the extend of the z dimension (preset is 1)
    */
    void XYAxisFlipImage( float* imageData, float* &flippedData, int xAxis, int yAxis, int dimension = 1 );

    ToFCameraPMDController::Pointer m_Controller; ///< corresponding CameraController
    ThreadedToFRawDataReconstruction::Pointer m_RawDataSource;
    char** m_SourceDataBuffer; ///< buffer holding the last acquired images
    char* m_SourceDataArray; ///< array holding the current PMD source data
    short* m_ShortSourceData; ///< array holding the current PMD raw data
  private:

    //member variables
    int m_ChannelSize;      ///< member holds the size of a single raw data channel

  };
} //END mitk namespace
#endif
