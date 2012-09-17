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
#ifndef __mitkToFCameraMESAController_h
#define __mitkToFCameraMESAController_h

#include "mitkMESASR4000ModuleExports.h"
#include "mitkCommon.h"

#include "itkObject.h"
#include "itkObjectFactory.h"

namespace mitk
{
  /**
  * @brief Virtual interface and base class for all MESA Time-of-Flight devices. Wraps MESA API provided in library
  * Provides methods for accessing current distance, amplitude, intensity and raw data.
  * Allows to set parameters like modulation frequency and integration time.
  *
  * @ingroup ToFHardware
  */
  class MITK_MESASR4000MODULE_EXPORT ToFCameraMESAController : public itk::Object
  {
  public:

    mitkClassMacro( ToFCameraMESAController , itk::Object );

    /*!
    \brief opens a connection to the ToF camera. Has to be implemented by the sub class
    */
    //TODO return value
    virtual bool OpenCameraConnection() = 0;
    /*!
    \brief closes the connection to the camera
    */
    //TODO return value
    virtual bool CloseCameraConnection();
    /*!
    \brief Gets the current amplitude array from the device
    \param amplitudeArray float array where the amplitude data will be saved
    */
    virtual bool GetAmplitudes(float* amplitudeArray);
    /*!
    \brief Gets the current intensity array from the device
    \param intensityArray float array where the intensity data will be saved
    */
    virtual bool GetIntensities(float* intensityArray);
    /*!
    \brief Gets the current distance array from the device
    \param distanceArray float array where the distance data will be saved
    */
    virtual bool GetDistances(float* distanceArray);
    /*!
    \brief calls update on the camera -> a new ToF-image is aquired
    */
    virtual bool UpdateCamera();
    /*!
    \brief Returns the currently set modulation frequency.
    \return modulation frequency
    */
    virtual int GetModulationFrequency() = 0;
    /*!
    \brief Sets the modulation frequency of the ToF device.
    The method automatically calculates a valid value from the given frequency to
    make sure that only valid frequencies are used.
    \param modulationFrequency modulation frequency
    \return frequency set after validation step
    */
    virtual int SetModulationFrequency(unsigned int modulationFrequency) = 0;
    /*!
    \brief Returns the currently set integration time.
    \return integration time
    */
    virtual int GetIntegrationTime() = 0;
    /*!
    \brief Sets the integration time of the ToF device.
    The method automatically calculates a valid value from the given integration time to
    make sure that only valid times are used.
    \param integrationTime integration time
    \return integration time set after validation step
    */
    virtual int SetIntegrationTime(unsigned int integrationTime) = 0;
    /*!
    \brief set input file name used by MESA player classes
    */
    virtual void SetInputFileName(std::string inputFileName);
    /*!
    \brief Access the resolution of the image in x direction
    \return widht of image in pixel
    */
    itkGetMacro(CaptureWidth, unsigned int);
    /*!
    \brief Access the resolution of the image in y direction
    \return height of image in pixel
    */
    itkGetMacro(CaptureHeight, unsigned int);

  protected:

    ToFCameraMESAController();

    ~ToFCameraMESAController();

    /*!
    \brief Method printing the current error message to the console and returning whether the previous command was successful
    \param error error number returned by the MESA function
    \return flag indicating if an error occured (false) or not (true)
    */
    bool ErrorText(int error);

    char m_MESAError[128]; ///< member holding the current error text
    int m_MESARes; ///< holds the current result message provided by MESA

    int m_PixelNumber; ///< holds the number of pixels contained in the image
    int m_NumberOfBytes; ///< holds the number of bytes contained in the image
    unsigned int m_CaptureWidth; ///< holds the width of the image in pixel
    unsigned int m_CaptureHeight; ///< holds the height of the image in pixel

    int m_NumImg; ///< holds the number of images the camera provided (distance, intensity, etc)

    float m_MaxRangeFactor; ///< holds the factor to calculate the real distance depends on the modulation frequency

    bool m_ConnectionCheck; ///< flag showing whether the camera is connected (true) or not (false)

    std::string m_InputFileName; ///< input file name used by MESA player classes

  private:

  };
} //END mitk namespace
#endif
