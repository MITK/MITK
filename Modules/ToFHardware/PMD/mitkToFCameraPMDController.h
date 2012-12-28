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
#ifndef __mitkToFCameraPMDController_h
#define __mitkToFCameraPMDController_h

#include "mitkPMDModuleExports.h"
#include "mitkCommon.h"


#include "itkObject.h"
#include "itkObjectFactory.h"

namespace mitk
{
  /**
  * @brief Virtual interface and base class for all PMD Time-of-Flight devices. Wraps PMD API provided in PMDSDK2
  * Provides methods for accessing current distance, amplitude, intensity and raw data.
  * Allows to set parameters like modulation frequency and integration time.
  *
  * @ingroup ToFHardware
  */
  class MITK_PMDMODULE_EXPORT ToFCameraPMDController : public itk::Object
  {
  public:

    mitkClassMacro( ToFCameraPMDController , itk::Object );

    /*!
    \brief opens a connection to the ToF camera. Has to be implemented by the sub class
    \throws mitkException In case of no connection, an exception is thrown!
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
    \brief Calculates the current amplitude data from the raw source data using the processing plugin of the PMDSDK
    \param sourceData raw data array
    \param amplitudeArray float array where the amplitude data will be saved
    */
    virtual bool GetAmplitudes(char* sourceData, float* amplitudeArray);
    /*!
    \brief Gets the current intensity array from the device
    \param intensityArray float array where the intensity data will be saved
    */
    virtual bool GetIntensities(float* intensityArray);
    /*!
    \brief Calculates the current intensity data from the raw source data using the processing plugin of the PMDSDK
    \param sourceData raw data array
    \param intensityArray float array where the intensity data will be saved
    */
    virtual bool GetIntensities(char* sourceData, float* intensityArray);
    /*!
    \brief Gets the current distance array from the device
    \param distanceArray float array where the distance data will be saved
    */
    virtual bool GetDistances(float* distanceArray);
    /*!
    \brief Calculates the current distance data from the raw source data using the processing plugin of the PMDSDK
    \param sourceData raw data array
    \param distanceArray float array where the distance data will be saved
    */
    virtual bool GetDistances(char* sourceData, float* distanceArray);
    /*!
    \brief Gets the PMD raw data from the ToF device.
    \param sourceDataArray array where the raw data will be saved
    */
    virtual bool GetSourceData(char* sourceDataArray);
    /*!
    \brief Convenience method to get the PMD raw data from the ToF device as short array.
    \param sourceData array where the short raw data will be saved
    */
    bool GetShortSourceData(short* sourceData);

    /*!
    \brief calls update on the camera -> a new ToF-image is aquired
    \throws mitkException In case of no connection, an exception is thrown!
    */
    virtual bool UpdateCamera();
    /*!
    \brief Returns the currently set modulation frequency.
    \return modulation frequency
    */
    virtual int GetModulationFrequency();
    /*!
    \brief Sets the modulation frequency of the ToF device.
    The method automatically calculates a valid value from the given frequency to
    make sure that only valid frequencies are used.
    \param modulationFrequency modulation frequency
    \return frequency set after validation step
    */
    virtual int SetModulationFrequency(unsigned int modulationFrequency);
    /*!
    \brief Returns the currently set integration time.
    \return integration time
    */
    virtual int GetIntegrationTime();
    /*!
    \brief Sets the integration time of the ToF device.
    The method automatically calculates a valid value from the given integration time to
    make sure that only valid times are used.
    \param integrationTime integration time
    \return integration time set after validation step
    */
    virtual int SetIntegrationTime(unsigned int integrationTime);
    /*!
    \brief set input file name used by PMD player classes
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

    itkGetMacro(SourceDataStructSize, int);

  protected:

    ToFCameraPMDController();

    ~ToFCameraPMDController();

    /*!
    \brief Method printing the current error message to the console and returning whether the previous command was successful
    \param error error number returned by the PMD function
    \return flag indicating if an error occured (false) or not (true)
    */
    bool ErrorText(int error);
    char m_PMDError[128]; ///< member holding the current error text
    int m_PMDRes; ///< holds the current result message provided by PMD

    int m_PixelNumber; ///< holds the number of pixels contained in the image
    int m_NumberOfBytes; ///< holds the number of bytes contained in the image
    unsigned int m_CaptureWidth; ///< holds the width of the image in pixel
    unsigned int m_CaptureHeight; ///< holds the height of the image in pixel

    int m_SourceDataSize; ///< size of the original PMD source data
    int m_SourceDataStructSize; ///< size of the PMD source data struct and the PMD source data

    bool m_ConnectionCheck; ///< flag showing whether the camera is connected (true) or not (false)

    std::string m_InputFileName; ///< input file name used by PMD player classes

    char *m_SourcePlugin; ///< holds name of source plugin to be loaded (e.g. camcube3.W64.pap for CamCube 3.0 on Win64 platform)
    char *m_SourceParam; ///< holds source parameter(s)
    char *m_ProcPlugin; ///< holds name of processing plugin to be loaded (e.g. camcubeproc.W64.pap for CamCube 3.0 on Win64 platform)
    char *m_ProcParam; ///< holds processing parameter(s)

  private:

  };
} //END mitk namespace
#endif
