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


#ifndef MITK_PHOTOACOUSTICS_BEAMFORMING_DMAS_FILTER
#define MITK_PHOTOACOUSTICS_BEAMFORMING_DMAS_FILTER

#include "mitkImageToImageFilter.h"

namespace mitk {

  //##Documentation
  //## @brief
  //## @ingroup Process
  class BeamformingDMASFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(BeamformingDMASFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

    struct beamformingSettings
    {
      double Pitch = 0.0003; // [m]
      float SpeedOfSound = 1540; // [m/s]
      unsigned int SamplesPerLine = 2048;
      unsigned int ReconstructionLines = 128;
      double RecordTime = 0.00006; // [s]
      unsigned int TransducerElements = 128;
      enum DelayCalc { Linear, QuadApprox, Spherical };
      DelayCalc DelayCalculationMethod = QuadApprox;
    };

    void Configure(beamformingSettings settings);

  protected:

    BeamformingDMASFilter();

    ~BeamformingDMASFilter();

    virtual void GenerateInputRequestedRegion() override;

    virtual void GenerateOutputInformation() override;

    virtual void GenerateData() override;

    //##Description
    //## @brief Time when Header was last initialized
    itk::TimeStamp m_TimeOfHeaderInitialization;

    mitk::Image::Pointer BandpassFilter(mitk::Image::Pointer data);
    itk::Image<double,3U>::Pointer BPFunction(mitk::Image::Pointer reference, int width, int center);

    double* m_OutputData;
    double* m_InputData;
    double* m_InputDataPuffer;

    beamformingSettings m_Conf;
  };

} // namespace mitk

#endif MITK_PHOTOACOUSTICS_BEAMFORMING_DMAS_FILTER
