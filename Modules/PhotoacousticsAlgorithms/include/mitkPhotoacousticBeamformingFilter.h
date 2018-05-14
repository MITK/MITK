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

#ifndef MITK_PHOTOACOUSTICS_BEAMFORMING_FILTER
#define MITK_PHOTOACOUSTICS_BEAMFORMING_FILTER

#include "mitkImageToImageFilter.h"
#include <functional>
#include "./OpenCLFilter/mitkPhotoacousticOCLBeamformingFilter.h"
#include "mitkPhotoacousticBeamformingSettings.h"

namespace mitk {
  /*!
  * \brief Class implementing an mitk::ImageToImageFilter for beamforming on both CPU and GPU
  *
  *  The class must be given a configuration class instance of mitk::BeamformingSettings for beamforming parameters through mitk::BeamformingFilter::Configure(BeamformingSettings settings)
  *  Whether the GPU is used can be set in the configuration.
  *  For significant problems or important messages a string is written, which can be accessed via GetMessageString().
  */

  class BeamformingFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(BeamformingFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /** \brief Sets a new configuration to use
    *
    * @param settings The configuration set to use for beamforming
    */
    void Configure(BeamformingSettings settings)
    {
      m_ConfOld = m_Conf;
      m_Conf = settings;
    }
    
    /** \brief Sets a new configuration to use
    * 
    *  The Filter writes important messages that can be retrieved through this method; if nothing is to be reported, it returns "noMessage".
    *  @return The message
    */
    std::string GetMessageString()
    {
      return m_Message;
    }

    /** \brief Sets a callback for progress checking
    *
    *  An std::function<void(int, std::string)> can be set, through which progress of the currently updating filter is reported.
    *  The integer argument is a number between 0 an 100 to indicate how far completion has been achieved, the std::string argument indicates what the filter is currently doing.
    */
    void SetProgressHandle(std::function<void(int, std::string)> progressHandle);

  protected:
    BeamformingFilter();

    ~BeamformingFilter() override;

    void GenerateInputRequestedRegion() override;

    void GenerateOutputInformation() override;

    void GenerateData() override;

    //##Description
    //## @brief Time when Header was last initialized
    itk::TimeStamp m_TimeOfHeaderInitialization;

    /** \brief The std::function<void(int, std::string)>, through which progress of the currently updating filter is reported.
    */
    std::function<void(int, std::string)> m_ProgressHandle;

    /** \brief Pointer holding the Von-Hann apodization window for beamforming
    * @param samples the resolution at which the window is created
    */
    float* VonHannFunction(int samples);
    /** \brief Function to create a Hamming apodization window
    * @param samples the resolution at which the window is created
    */
    float* HammFunction(int samples);
    /** \brief Function to create a Box apodization window
    * @param samples the resolution at which the window is created
    */
    float* BoxFunction(int samples);

    /** \brief Function to perform beamforming on CPU for a single line, using DAS and quadratic delay
    */
    void DASQuadraticLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize);
    /** \brief Function to perform beamforming on CPU for a single line, using DAS and spherical delay
    */
    void DASSphericalLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize);
    /** \brief Function to perform beamforming on CPU for a single line, using DMAS and quadratic delay
    */
    void DMASQuadraticLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize);
    /** \brief Function to perform beamforming on CPU for a single line, using DMAS and spherical delay
    */
    void DMASSphericalLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize);
    /** \brief Function to perform beamforming on CPU for a single line, using signed DMAS and quadratic delay
    */
    void sDMASQuadraticLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize);
    /** \brief Function to perform beamforming on CPU for a single line, using signed DMAS and spherical delay
    */
    void sDMASSphericalLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize);

    float* m_OutputData;
    float* m_InputData;
    float* m_InputDataPuffer;

    /** \brief Pointer holding the Von-Hann apodization window for beamforming
    */
    float* m_VonHannFunction;
    /** \brief Pointer holding the Hamming apodization window for beamforming
    */
    float* m_HammFunction;
    /** \brief Pointer holding the Box apodization window for beamforming
    */
    float* m_BoxFunction;

    /** \brief Current configuration set
    */
    BeamformingSettings m_Conf;
    /** \brief Previous configuration set to selectively update used data for beamforming
    */
    BeamformingSettings m_ConfOld;

    /** \brief Pointer to the GPU beamforming filter class; for performance reasons the filter is initialized within the constructor and kept for all later computations.
    */
    mitk::PhotoacousticOCLBeamformingFilter::Pointer m_BeamformingOclFilter;

    /** \brief The message returned by mitk::BeamformingFilter::GetMessageString()
    */
    std::string m_Message;
  };
} // namespace mitk

#endif //MITK_PHOTOACOUSTICS_BEAMFORMING_FILTER
