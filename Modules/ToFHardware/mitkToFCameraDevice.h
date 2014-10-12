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
#ifndef __mitkToFCameraDevice_h
#define __mitkToFCameraDevice_h

#include <MitkToFHardwareExports.h>
#include "mitkCommon.h"
#include "mitkStringProperty.h"
#include "mitkProperties.h"
#include "mitkPropertyList.h"

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkMultiThreader.h"
#include "itkFastMutexLock.h"

// Microservices
#include <mitkServiceInterface.h>

namespace mitk
{
  /**
  * @brief Virtual interface and base class for all Time-of-Flight devices.
  *
  * @ingroup ToFHardware
  */
  class MITK_TOFHARDWARE_EXPORT ToFCameraDevice : public itk::Object
  {
  public:

    mitkClassMacro(ToFCameraDevice, itk::Object);
    /*!
    \brief Opens a connection to the ToF camera. Has to be implemented
    in the specialized inherited classes.
    \return True for success.
    */
    virtual bool OnConnectCamera() = 0;

    /**
     * @brief ConnectCamera Internally calls OnConnectCamera() of the
     * respective device implementation.
     * @return True for success.
     */
    virtual bool ConnectCamera();
    /*!
    \brief closes the connection to the camera
    */
    virtual bool DisconnectCamera() = 0;
    /*!
    \brief starts the continuous updating of the camera.
    A separate thread updates the source data, the main thread processes the source data and creates images and coordinates
    */
    virtual void StartCamera() = 0;
    /*!
    \brief stops the continuous updating of the camera
    */
    virtual void StopCamera();
    /*!
    \brief returns true if the camera is connected and started
    */
    virtual bool IsCameraActive();
    /*!
    \brief returns true if the camera is connected
    */
    virtual bool IsCameraConnected();
    /*!
    \brief updates the camera for image acquisition
    */
    virtual void UpdateCamera() = 0;
    /*!
    \brief gets the amplitude data from the ToF camera as the strength of the active illumination of every pixel
    These values can be used to determine the quality of the distance values. The higher the amplitude value, the higher the accuracy of the according distance value
    \param imageSequence the actually captured image sequence number
    \param amplitudeArray contains the returned amplitude data as an array.
    */
    virtual void GetAmplitudes(float* amplitudeArray, int& imageSequence) = 0;
    /*!
    \brief gets the intensity data from the ToF camera as a greyscale image
    \param intensityArray contains the returned intensities data as an array.
    \param imageSequence the actually captured image sequence number
    */
    virtual void GetIntensities(float* intensityArray, int& imageSequence) = 0;
    /*!
    \brief gets the distance data from the ToF camera measuring the distance between the camera and the different object points in millimeters
    \param distanceArray contains the returned distances data as an array.
    \param imageSequence the actually captured image sequence number
    */
    virtual void GetDistances(float* distanceArray, int& imageSequence) = 0;
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
                              int requiredImageSequence, int& capturedImageSequence, unsigned char* rgbDataArray=NULL) = 0;
    /*!
    \brief get the currently set capture width
    \return capture width
    */
    itkGetMacro(CaptureWidth, int);
    /*!
    \brief get the currently set capture height
    \return capture height
    */
    itkGetMacro(CaptureHeight, int);
    /*!
    \brief get the currently set source data size
    \return source data size
    */
    itkGetMacro(SourceDataSize, int);
    /*!
    \brief get the currently set buffer size
    \return buffer size
    */
    itkGetMacro(BufferSize, int);
    /*!
    \brief get the currently set max buffer size
    \return max buffer size
    */
    itkGetMacro(MaxBufferSize, int);

    /*!
    \brief set a bool property in the property list
    */
    void SetBoolProperty( const char* propertyKey, bool boolValue );

    /*!
    \brief set an int property in the property list
    */
    void SetIntProperty( const char* propertyKey, int intValue );

    /*!
    \brief set a float property in the property list
    */
    void SetFloatProperty( const char* propertyKey, float floatValue );

    /*!
    \brief set a string property in the property list
    */
    void SetStringProperty( const char* propertyKey, const char* stringValue );

    /*!
    \brief set a BaseProperty property in the property list
    */
    virtual void SetProperty( const char *propertyKey, BaseProperty* propertyValue );

    /*!
    \brief get a BaseProperty from the property list
    */
    virtual BaseProperty* GetProperty( const char *propertyKey );

    /*!
    \brief get a bool from the property list
    */
    bool GetBoolProperty(const char *propertyKey, bool& boolValue);

    /*!
    \brief get a string from the property list
    */
    bool GetStringProperty(const char *propertyKey, std::string& string);

    /*!
    \brief get an int from the property list
    */
    bool GetIntProperty(const char *propertyKey, int& integer);

    virtual int GetRGBCaptureWidth();

    virtual int GetRGBCaptureHeight();

  protected:

    ToFCameraDevice();

    ~ToFCameraDevice();

    /*!
    \brief method for allocating memory for pixel arrays m_IntensityArray, m_DistanceArray and m_AmplitudeArray
    */
    virtual void AllocatePixelArrays();
    /*!
    \brief method for cleanup memory allocated for pixel arrays m_IntensityArray, m_DistanceArray and m_AmplitudeArray
    */
    virtual void CleanupPixelArrays();

    float* m_IntensityArray; ///< float array holding the intensity image
    float* m_DistanceArray; ///< float array holding the distance image
    float* m_AmplitudeArray; ///< float array holding the amplitude image
    int m_BufferSize; ///< buffer size of the image buffer needed for loss-less acquisition of range data
    int m_MaxBufferSize; ///< maximal buffer size needed for initialization of data arrays. Default value is 100.
    int m_CurrentPos; ///< current position in the buffer which will be retrieved by the Get methods
    int m_FreePos; ///< current position in the buffer which will be filled with data acquired from the hardware
    int m_CaptureWidth; ///< width of the range image (x dimension)
    int m_CaptureHeight; ///< height of the range image (y dimension)
    int m_PixelNumber; ///< number of pixels in the range image (m_CaptureWidth*m_CaptureHeight)
    int m_RGBImageWidth; ///< width of the RGB image (x dimension)
    int m_RGBImageHeight; ///< height of the RGB image (y dimension)
    int m_RGBPixelNumber; ///< number of pixels in the range image (m_RGBImageWidth*m_RGBImageHeight)
    int m_SourceDataSize; ///< size of the PMD source data
    itk::MultiThreader::Pointer m_MultiThreader; ///< itk::MultiThreader used for thread handling
    itk::FastMutexLock::Pointer m_ImageMutex; ///< mutex for images provided by the range camera
    itk::FastMutexLock::Pointer m_CameraActiveMutex; ///< mutex for the cameraActive flag
    int m_ThreadID; ///< ID of the started thread
    bool m_CameraActive; ///< flag indicating if the camera is currently active or not. Caution: thread safe access only!
    bool m_CameraConnected; ///< flag indicating if the camera is successfully connected or not. Caution: thread safe access only!
    int m_ImageSequence; ///<  counter for acquired images

    PropertyList::Pointer m_PropertyList; ///< a list of the corresponding properties

  };
} //END mitk namespace
/**
ToFCameraDevice is declared a MicroService interface. See
MicroService documenation for more details.
*/
MITK_DECLARE_SERVICE_INTERFACE(mitk::ToFCameraDevice, "org.mitk.services.ToFCameraDevice")
#endif
