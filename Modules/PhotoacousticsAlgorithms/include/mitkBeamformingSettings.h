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

#ifndef MITK_BEAMFORMING_SETTINGS
#define MITK_BEAMFORMING_SETTINGS

#include <itkObject.h>
#include <itkMacro.h>
#include <mitkCommon.h>
#include <MitkPhotoacousticsAlgorithmsExports.h>

namespace mitk {
  /*!
  * \brief Class holding the configuration data for the beamforming filters mitk::BeamformingFilter and mitk::PhotoacousticOCLBeamformingFilter
  *
  * A detailed description can be seen below. All parameters should be set manually for successfull beamforming.
  */

  class MITKPHOTOACOUSTICSALGORITHMS_EXPORT BeamformingSettings : public itk::Object
  {
  public:
    mitkClassMacroItkParent(BeamformingSettings, itk::Object);
    itkCloneMacro(Self);

    /** \brief Available delay calculation methods:
    * - Spherical delay for best results.
    * - DEPRECATED quadratic Taylor approximation for slightly faster results with hardly any quality loss.
    */
    enum DelayCalc { QuadApprox, Spherical };

    /** \brief Available apodization functions:
    * - Hamming function.
    * - Von-Hann function.
    * - Box function.
    */
    enum Apodization { Hamm, Hann, Box };

    /** \brief Available beamforming algorithms:
    * - DAS (Delay and sum).
    * - DMAS (Delay multiply and sum).
    */
    enum BeamformingAlgorithm { DMAS, DAS, sDMAS };

    itkGetConstMacro(PitchInMeters, float);
    itkGetConstMacro(SpeedOfSound, float);
    itkGetConstMacro(TimeSpacing, float);
    itkGetConstMacro(Angle, float);
    itkGetConstMacro(IsPhotoacousticImage, bool);
    itkGetConstMacro(TransducerElements, unsigned int);
    itkGetConstMacro(SamplesPerLine, unsigned int);
    itkGetConstMacro(ReconstructionLines, unsigned int);
    itkGetConstMacro(InputDim, const unsigned int*);
    itkGetConstMacro(UseGPU, bool);
    itkGetConstMacro(GPUBatchSize, unsigned int);
    itkGetConstMacro(DelayCalculationMethod, DelayCalc);
    itkGetConstMacro(ApodizationFunction, const float*);
    itkGetConstMacro(Apod, Apodization);
    itkGetConstMacro(ApodizationArraySize, int);
    itkGetConstMacro(Algorithm, BeamformingAlgorithm);
    itkGetConstMacro(ReconstructionDepth, float);

    /** \brief function for mitk::PhotoacousticOCLBeamformingFilter to check whether buffers need to be updated
    * this method only checks parameters relevant for the openCL implementation
    */
    static bool SettingsChangedOpenCL(const BeamformingSettings::Pointer lhs, const BeamformingSettings::Pointer rhs)
    {
      return !((abs(lhs->GetAngle() - rhs->GetAngle()) < 0.01f) && // 0.01 degree error margin
        (lhs->GetApod() == rhs->GetApod()) &&
        (lhs->GetDelayCalculationMethod() == rhs->GetDelayCalculationMethod()) &&
        (lhs->GetIsPhotoacousticImage() == rhs->GetIsPhotoacousticImage()) &&
        (abs(lhs->GetPitchInMeters() - rhs->GetPitchInMeters()) < 0.000001f) && // 0.0001 mm error margin
        (lhs->GetReconstructionLines() == rhs->GetReconstructionLines()) &&
        (lhs->GetSamplesPerLine() == rhs->GetSamplesPerLine()) &&
        (lhs->GetReconstructionDepth() == rhs->GetReconstructionDepth()) &&
        (abs(lhs->GetSpeedOfSound() - rhs->GetSpeedOfSound()) < 0.01f) &&
        (abs(lhs->GetTimeSpacing() - rhs->GetTimeSpacing()) < 0.00000000001f) && //0.01 ns error margin
        (lhs->GetTransducerElements() == rhs->GetTransducerElements()));
    }

    static Pointer New(float pitchInMeters,
      float speedOfSound,
      float timeSpacing,
      float angle,
      bool isPhotoacousticImage,
      unsigned int samplesPerLine,
      unsigned int reconstructionLines,
      unsigned int* inputDim,
      float reconstructionDepth,
      bool useGPU,
      unsigned int GPUBatchSize,
      DelayCalc delayCalculationMethod,
      Apodization apod,
      unsigned int apodizationArraySize,
      BeamformingAlgorithm algorithm)
    {
      Pointer smartPtr = new BeamformingSettings(pitchInMeters,
        speedOfSound,
        timeSpacing,
        angle,
        isPhotoacousticImage,
        samplesPerLine,
        reconstructionLines,
        inputDim,
        reconstructionDepth,
        useGPU,
        GPUBatchSize,
        delayCalculationMethod,
        apod,
        apodizationArraySize,
        algorithm);
      smartPtr->UnRegister();
      return smartPtr;
    }

  protected:

    /**
    */
    BeamformingSettings(float pitchInMeters,
      float speedOfSound,
      float timeSpacing,
      float angle,
      bool isPhotoacousticImage,
      unsigned int samplesPerLine,
      unsigned int reconstructionLines,
      unsigned int* inputDim,
      float reconstructionDepth,
      bool useGPU,
      unsigned int GPUBatchSize,
      DelayCalc delayCalculationMethod,
      Apodization apod,
      unsigned int apodizationArraySize,
      BeamformingAlgorithm algorithm
    );

    ~BeamformingSettings();

    /** \brief Pitch of the used transducer in [m].
    */
    float m_PitchInMeters;

    /** \brief Speed of sound in the used medium in [m/s].
    */
    float m_SpeedOfSound;

    /** \brief The time spacing of the input image
    */
    float m_TimeSpacing; // [s]

    /** \brief The angle of the transducer elements
    */
    float m_Angle;

    /** \brief Flag whether processed image is a photoacoustic image or an ultrasound image
    */
    bool m_IsPhotoacousticImage;

    /** \brief How many transducer elements the used transducer had.
    */
    unsigned int m_TransducerElements;

    /** \brief How many vertical samples should be used in the final image.
    */
    unsigned int m_SamplesPerLine;

    /** \brief How many lines should be reconstructed in the final image.
    */
    unsigned int m_ReconstructionLines;

    /** \brief Sets the dimensions of the inputImage.
    */
    const unsigned int* m_InputDim;

    /** \brief The Depth up to which the filter should reconstruct the image [m]
    */
    float m_ReconstructionDepth;

    /** \brief Decides whether GPU computing should be used
    */
    bool m_UseGPU;

    unsigned int m_GPUBatchSize;
    /** \brief Sets the amount of image slices in batches when GPU is used
    */

    /** \brief Sets how the delays for beamforming should be calculated.
    */
    DelayCalc m_DelayCalculationMethod;

    const float* m_ApodizationFunction;

    /** \brief Sets the used apodization function.
    */
    Apodization m_Apod;

    /** \brief Sets the resolution of the apodization array (must be greater than 0).
    */
    int m_ApodizationArraySize;

    /** \brief Sets the used beamforming algorithm.
    */
    BeamformingAlgorithm m_Algorithm;
  };
}
#endif //MITK_BEAMFORMING_SETTINGS
