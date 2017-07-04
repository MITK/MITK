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

    struct beamformingSettings
    {
      float Pitch = 0.0003; // [m]
      float SpeedOfSound = 1540; // [m/s]
      unsigned int SamplesPerLine = 2048;
      unsigned int ReconstructionLines = 128;
      float RecordTime = 0.00006; // [s]
      unsigned int TransducerElements = 128;
      bool partial = false;
      unsigned int bounds[2] = { 0,0 };

      bool UseGPU = true;

      enum DelayCalc {QuadApprox, Spherical};
      DelayCalc DelayCalculationMethod = QuadApprox;

      enum Apodization {Hamm, Hann, Box};
      Apodization Apod = Hann;

      enum BeamformingAlgorithm {DMAS, DAS};
      BeamformingAlgorithm Algorithm = DAS;

      float Angle = 10;
      bool Photoacoustic = true;
      float BPHighPass = 50;
      float BPLowPass = 50;
      bool UseBP = false;
      unsigned int ButterworthOrder = 8;
    };

    void Configure(beamformingSettings settings);

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

    mitk::Image::Pointer BandpassFilter(mitk::Image::Pointer data);
    itk::Image<float, 3U>::Pointer BPFunction(mitk::Image::Pointer reference, int width, int center);

    float* m_OutputData;
    float* m_InputData;
    float* m_InputDataPuffer;

    beamformingSettings m_Conf;
  };

} // namespace mitk

#endif MITK_PHOTOACOUSTICS_BEAMFORMING_FILTER
