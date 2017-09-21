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
    unsigned int upperCutoff = 0;

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

    //this method ignores changes in BPLow/BPHigh/cropBounds/Algorithm/some more, as those are insignifiant in all current situations
    static bool SettingsChangedOpenCL(const BeamformingSettings& lhs, const BeamformingSettings& rhs)
    {
      return !(((lhs.Angle - rhs.Angle) < 0.001f) &&
        (lhs.Apod == rhs.Apod) &&
        (lhs.DelayCalculationMethod == rhs.DelayCalculationMethod) &&
        (lhs.isPhotoacousticImage == rhs.isPhotoacousticImage) &&
        ((lhs.Pitch - rhs.Pitch) < 0.0000001f) &&
        (lhs.ReconstructionLines == rhs.ReconstructionLines) &&
        (lhs.RecordTime - rhs.RecordTime) < 0.00000001f &&
        (lhs.SamplesPerLine == rhs.SamplesPerLine) &&
        ((lhs.SpeedOfSound - rhs.SpeedOfSound) < 0.01f) &&
        ((lhs.TimeSpacing - rhs.TimeSpacing) < 0.0000000001f) &&
        (lhs.TransducerElements == rhs.TransducerElements));
    }
  };
}
#endif