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
#ifndef __mitkToFImageGrabber_h
#define __mitkToFImageGrabber_h

#include <MitkToFHardwareExports.h>
#include "mitkCommon.h"
#include "mitkImageSource.h"
#include "mitkToFCameraDevice.h"

#include "itkObject.h"
#include "itkObjectFactory.h"

namespace mitk
{
  /**Documentation
  * \brief Image source providing ToF images. Interface for filters provided in ToFProcessing module
  *
  * This class internally holds a ToFCameraDevice to access the images acquired by a ToF camera.
  *
  * Provided images include: distance image (output 0), amplitude image (output 1), intensity image (output 2)
  *
  * \ingroup ToFHardware
  */
  class MITK_TOFHARDWARE_EXPORT ToFImageGrabber : public mitk::ImageSource
  {
  public:

    mitkClassMacro( ToFImageGrabber , ImageSource );

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void ShowDebugImage(float* distances);
    /*!
    \brief Establish a connection to the ToF camera
    */
    virtual bool ConnectCamera();
    /*!
    \brief Disconnects the ToF camera
    */
    virtual bool DisconnectCamera();
    /*!
    \brief Starts the continuous updating of the camera.
    A separate thread updates the source data, the main thread processes the source data and creates images and coordinates
    */
    virtual void StartCamera();
    /*!
    \brief Stops the continuous updating of the camera
    */
    virtual void StopCamera();
    /*!
    \brief Returns true if the camera is connected and started
    */
    virtual bool IsCameraActive();
    /*!
    \brief Returns true if the camera is connected
    */
    virtual bool IsCameraConnected();
    /*!
    \brief Sets the ToF device, the image grabber is grabbing the images from
    \param aToFCameraDevice device internally used for grabbing the images from the camera
    */
    void SetCameraDevice(ToFCameraDevice* aToFCameraDevice);
    /*!
    \brief Get the currently set ToF camera device
    \return device currently used for grabbing images from the camera
    */
    ToFCameraDevice* GetCameraDevice();
    /*!
    \brief Set the modulation frequency used by the ToF camera.
    For default values see the corresponding device classes
    \param modulationFrequency modulation frequency in Hz
    */
    int SetModulationFrequency(int modulationFrequency);
    /*!
    \brief Get the modulation frequency used by the ToF camera.
    \return modulation frequency in MHz
    */
    int GetModulationFrequency();
    /*!
    \brief Set the integration time used by the ToF camera.
    For default values see the corresponding device classes
    \param integrationTime integration time in ms
    */
    int SetIntegrationTime(int integrationTime);
    /*!
    \brief Get the integration time in used by the ToF camera.
    \return integration time in ms
    */
    int GetIntegrationTime();
    /*!
    \brief Get the dimension in x direction of the ToF image
    \return width of the image
    */
    int GetCaptureWidth();
    /*!
    \brief Get the dimension in y direction of the ToF image
    \return height of the image
    */
    int GetCaptureHeight();
    /*!
    \brief Get the number of pixel in the ToF image
    \return number of pixel
    */
    int GetPixelNumber();
    /*!
    \brief Get the dimension in x direction of the ToF image
    \return width of the image
    */
    int GetRGBImageWidth();
    /*!
    \brief Get the dimension in y direction of the ToF image
    \return height of the image
    */
    int GetRGBImageHeight();
    /*!
    \brief Get the number of pixel in the ToF image
    \return number of pixel
    */
    int GetRGBPixelNumber();

// properties
    void SetBoolProperty( const char* propertyKey, bool boolValue );

    void SetIntProperty( const char* propertyKey, int intValue );

    void SetFloatProperty( const char* propertyKey, float floatValue );

    void SetStringProperty( const char* propertyKey, const char* stringValue );

    void SetProperty( const char *propertyKey, BaseProperty* propertyValue );

    bool GetBoolProperty( const char* propertyKey);

    int GetIntProperty( const char* propertyKey);

    float GetFloatProperty( const char* propertyKey);

    const char* GetStringProperty( const char* propertyKey);

    BaseProperty* GetProperty( const char *propertyKey);


  protected:

    ///
    /// called when the ToFCameraDevice was modified
    ///
    void OnToFCameraDeviceModified();

    /*!
    \brief clean up memory allocated for the image arrays m_IntensityArray, m_DistanceArray, m_AmplitudeArray and m_SourceDataArray
    */
    virtual void CleanUpImageArrays();
    /*!
    \brief Allocate memory for the image arrays m_IntensityArray, m_DistanceArray, m_AmplitudeArray and m_SourceDataArray
    */
    virtual void AllocateImageArrays();

    /**
     * @brief InitializeImages Initialze the geometries of the images according to the device properties.
     */
    void InitializeImages();

    ToFCameraDevice::Pointer m_ToFCameraDevice; ///< Device allowing access to ToF image data
    int m_CaptureWidth; ///< Width of the captured ToF image
    int m_CaptureHeight; ///< Height of the captured ToF image
    int m_PixelNumber; ///< Number of pixels in the image
    int m_RGBImageWidth; ///< Width of the captured RGB image
    int m_RGBImageHeight; ///< Height of the captured RGB image
    int m_RGBPixelNumber; ///< Number of pixels in the RGB image
    int m_ImageSequence; ///< counter for currently acquired images
    int m_SourceDataSize; ///< size of the source data in bytes
    float* m_IntensityArray; ///< member holding the current intensity array
    float* m_DistanceArray; ///< member holding the current distance array
    float* m_AmplitudeArray; ///< member holding the current amplitude array
    char* m_SourceDataArray;///< member holding the current source data array
    unsigned char* m_RgbDataArray; ///< member holding the current rgb data array
    unsigned long m_DeviceObserverTag; ///< tag of the observer for the ToFCameraDevice
    ToFImageGrabber();

    ~ToFImageGrabber();

    /*!
    \brief Method generating the outputs of this filter. Called in the updated process of the pipeline.
    0: distance image
    1: amplitude image
    2: intensity image
    3: RGB image
    */
    void GenerateData();

  private:

  };
} //END mitk namespace
#endif
