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
  //##Documentation
  //## @brief
  //## @ingroup Process

  class BeamformingFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(BeamformingFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

    void Configure(BeamformingSettings settings);

    void SetProgressHandle(std::function<void(int, std::string)> progressHandle);

  protected:

    BeamformingFilter();

    ~BeamformingFilter();

    virtual void GenerateInputRequestedRegion() override;

    virtual void GenerateOutputInformation() override;

    virtual void GenerateData() override;

    //##Description
    //## @brief Time when Header was last initialized
    itk::TimeStamp m_TimeOfHeaderInitialization;

    std::function<void(int, std::string)> m_ProgressHandle;

    float* VonHannFunction(int samples);
    float* HammFunction(int samples);
    float* BoxFunction(int samples);

    void DASQuadraticLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize);
    void DASSphericalLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize);

    void DMASQuadraticLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize);
    void DMASSphericalLine(float* input, float* output, float inputDim[2], float outputDim[2], const short& line, float* apodisation, const short& apodArraySize);

    float* m_OutputData;
    float* m_InputData;
    float* m_InputDataPuffer;

    BeamformingSettings m_Conf;

    mitk::PhotoacousticOCLBeamformingFilter::Pointer m_BeamformingOclFilter;
  };
} // namespace mitk

#endif //MITK_PHOTOACOUSTICS_BEAMFORMING_FILTER
