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

#ifndef MITK_PHOTOACOUSTICS_BEAMFORMING_SETTINGS
#define MITK_PHOTOACOUSTICS_BEAMFORMING_SETTINGS

namespace mitk {
  //##Documentation
  //## @brief
  //## @ingroup Process

  class BeamformingSettings
  {
  public:
    float Pitch = 0.0003; // [m]
    float SpeedOfSound = 1540; // [m/s]
    unsigned int SamplesPerLine = 2048;
    unsigned int ReconstructionLines = 128;
    float RecordTime = 0.00006; // [s]
    float TimeSpacing = 0.0000000000001; // [s]
    unsigned short TransducerElements = 128;
    bool partial = false;
    unsigned int CropBounds[2] = { 0,0 };
    unsigned int inputDim[3] = { 1,1,1 };

    bool UseGPU = true;

    enum DelayCalc { QuadApprox, Spherical };
    DelayCalc DelayCalculationMethod = QuadApprox;

    enum Apodization { Hamm, Hann, Box };
    Apodization Apod = Hann;

    enum BeamformingAlgorithm { DMAS, DAS };
    BeamformingAlgorithm Algorithm = DAS;

    float Angle = 10;
    bool isPhotoacousticImage = true;
    float BPHighPass = 50;
    float BPLowPass = 50;
    bool UseBP = false;

    friend bool operator==(const BeamformingSettings& lhs, const BeamformingSettings& rhs)
    {
      return (lhs.Algorithm == rhs.Algorithm) &&
        (lhs.Angle == rhs.Angle) &&
        (lhs.Apod == rhs.Apod) &&
        (lhs.BPHighPass == rhs.BPHighPass) &&
        (lhs.BPLowPass == rhs.BPLowPass) &&
        (lhs.CropBounds == rhs.CropBounds) &&
        (lhs.DelayCalculationMethod == rhs.DelayCalculationMethod) &&
        (lhs.isPhotoacousticImage == rhs.isPhotoacousticImage) &&
        (lhs.inputDim[0] == rhs.inputDim[0]) &&
        (lhs.inputDim[1] == rhs.inputDim[1]) &&
        (lhs.inputDim[2] == rhs.inputDim[2]) &&
        (lhs.partial == rhs.partial) &&
        (lhs.Pitch == rhs.Pitch) &&
        (lhs.ReconstructionLines == rhs.ReconstructionLines) &&
        (lhs.RecordTime == rhs.RecordTime) &&
        (lhs.SamplesPerLine == rhs.SamplesPerLine) &&
        (lhs.SpeedOfSound == rhs.SpeedOfSound) &&
        (lhs.TimeSpacing == rhs.TimeSpacing) &&
        (lhs.TransducerElements == rhs.TransducerElements) &&
        (lhs.UseBP == rhs.UseBP) &&
        (lhs.UseGPU == rhs.UseGPU);
    }
  };
}
#endif