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
  /*!
  * \brief Class holding the configuration data for the beamforming filters mitk::BeamformingFilter and mitk::PhotoacousticOCLBeamformingFilter
  *
  * A detailed description can be seen below. All parameters should be set manually for successfull beamforming.
  */

  class BeamformingSettings
  {
  public:
    /** \brief Pitch of the used transducer in [m].
    */
    float Pitch = 0.0003;
    /** \brief Speed of sound in the used medium in [m/s].
    */
    float SpeedOfSound = 1540;
    /** \brief This parameter is not neccessary to be set, as it's never used.
    */
    float RecordTime = 0.00006; // [s]
    /** \brief The time spacing of the input image
    */
    float TimeSpacing = 0.0000000000001; // [s]
    /** \brief The angle of the transducer elements
    */
    float Angle = -1;
    /** \brief Flag whether processed image is a photoacoustic image or an ultrasound image
    */
    bool isPhotoacousticImage = true;

    /** \brief How many transducer elements the used transducer had.
    */
    unsigned short TransducerElements = 128;
    /** \brief How many vertical samples should be used in the final image.
    */
    unsigned int SamplesPerLine = 2048;
    /** \brief How many lines should be reconstructed in the final image.
    */
    unsigned int ReconstructionLines = 128;
    /** \brief Sets how many voxels should be cut off from the top of the image before beamforming, to potentially avoid artifacts.
    */
    unsigned int upperCutoff = 0;
    /** \brief Sets whether only the slices selected by mitk::BeamformingSettings::CropBounds should be beamformed.
    */
    bool partial = false;
    /** \brief Sets the first and last slice to be beamformed.
    */
    unsigned int CropBounds[2] = { 0,0 };
    /** \brief Sets the dimensions of the inputImage.
    */
    unsigned int inputDim[3] = { 1,1,1 };

    /** \brief Decides whether GPU computing should be used
    */
    bool UseGPU = true;

    /** \brief Available delay calculation methods:
    * - Spherical delay for best results.
    * - A quadratic Taylor approximation for slightly faster results with hardly any quality loss.
    */
    enum DelayCalc { QuadApprox, Spherical };
    /** \brief Sets how the delays for beamforming should be calculated.
    */
    DelayCalc DelayCalculationMethod = QuadApprox;

    /** \brief Available apodization functions:
    * - Hamming function.
    * - Von-Hann function.
    * - Box function.
    */
    enum Apodization { Hamm, Hann, Box };
    /** \brief Sets the used apodization function.
    */
    Apodization Apod = Hann;
    /** \brief Sets the resolution of the apodization array (must be greater than 0).
    */
    int apodizationArraySize = 128;

    /** \brief Available beamforming algorithms:
    * - DAS (Delay and sum).
    * - DMAS (Delay multiply and sum).
    */
    enum BeamformingAlgorithm { DMAS, DAS, sDMAS};
    /** \brief Sets the used beamforming algorithm.
    */
    BeamformingAlgorithm Algorithm = DAS;

    /** \brief Sets whether after beamforming a bandpass should be automatically applied
    */
    bool UseBP = false;
    /** \brief Sets the position at which lower frequencies are completely cut off in Hz.
    */
    float BPHighPass = 50;
    /** \brief Sets the position at which higher frequencies are completely cut off in Hz.
    */
    float BPLowPass = 50;
    
    /** \brief function for mitk::PhotoacousticOCLBeamformingFilter to check whether buffers need to be updated
    * this method only checks parameters relevant for the openCL implementation
    */
    static bool SettingsChangedOpenCL(const BeamformingSettings& lhs, const BeamformingSettings& rhs)
    {
      return !((abs(lhs.Angle - rhs.Angle) < 0.01f) && // 0.01 degree error margin
        (lhs.Apod == rhs.Apod) &&
        (lhs.DelayCalculationMethod == rhs.DelayCalculationMethod) &&
        (lhs.isPhotoacousticImage == rhs.isPhotoacousticImage) &&
        (abs(lhs.Pitch - rhs.Pitch) < 0.000001f) && // 0.0001 mm error margin
        (lhs.ReconstructionLines == rhs.ReconstructionLines) &&
        (abs(lhs.RecordTime - rhs.RecordTime) < 0.00000001f) && // 10 ns error margin
        (lhs.SamplesPerLine == rhs.SamplesPerLine) &&
        (abs(lhs.SpeedOfSound - rhs.SpeedOfSound) < 0.01f) &&
        (abs(lhs.TimeSpacing - rhs.TimeSpacing) < 0.00000000001f) && //0.01 ns error margin
        (lhs.TransducerElements == rhs.TransducerElements));
    }
  };
}
#endif