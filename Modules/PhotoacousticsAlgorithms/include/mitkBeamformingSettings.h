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
    mitkNewMacro1Param(BeamformingSettings, std::string);
    itkCloneMacro(Self);

    /** \brief Available delay calculation methods:
    * - Spherical delay for best results.
    * - A quadratic Taylor approximation for slightly faster results with hardly any quality loss.
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
    itkGetConstMacro(UpperCutoff, unsigned int);
    itkGetConstMacro(Partial, bool);
    itkGetConstMacro(CropBounds, const unsigned int*);
    itkGetConstMacro(InputDim, const unsigned int*);
    itkGetConstMacro(UseGPU, bool);
    itkGetConstMacro(DelayCalculationMethod, DelayCalc);
    itkGetConstMacro(ApodizationFunction, const float*);
    itkGetConstMacro(Apod, Apodization);
    itkGetConstMacro(ApodizationArraySize, int);
    itkGetConstMacro(Algorithm, BeamformingAlgorithm);
    itkGetConstMacro(UseBP, bool);
    itkGetConstMacro(BPHighPass, float);
    itkGetConstMacro(BPLowPass, float);

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
        (abs(lhs->GetSpeedOfSound() - rhs->GetSpeedOfSound()) < 0.01f) &&
        (abs(lhs->GetTimeSpacing() - rhs->GetTimeSpacing()) < 0.00000000001f) && //0.01 ns error margin
        (lhs->GetTransducerElements() == rhs->GetTransducerElements()));
    }

    static Pointer New(float pitchInMeters,
      float speedOfSound,
      float timeSpacing,
      float angle,
      bool isPhotoacousticImage,
      unsigned int transducerElements,
      unsigned int upperCutoff,
      bool partial,
      unsigned int* cropBounds,
      unsigned int* inputDim,
      bool useGPU,
      DelayCalc delayCalculationMethod,
      Apodization apod,
      unsigned int apodizationArraySize,
      BeamformingAlgorithm algorithm,
      bool useBP,
      float BPHighPass,
      float BPLowPass)
    {
      Pointer smartPtr = new BeamformingSettings(pitchInMeters,
        speedOfSound,
        timeSpacing,
        angle,
        isPhotoacousticImage,
        transducerElements,
        upperCutoff,
        partial,
        cropBounds,
        inputDim,
        useGPU,
        delayCalculationMethod,
        apod,
        apodizationArraySize,
        algorithm,
        useBP,
        BPHighPass,
        BPLowPass);
      smartPtr->UnRegister();
      return smartPtr;
    }

  protected:

    /**
    */
    BeamformingSettings(std::string xmlFile);

    /**
    */
    BeamformingSettings(float pitchInMeters,
      float speedOfSound,
      float timeSpacing,
      float angle,
      bool isPhotoacousticImage,
      unsigned int transducerElements,
      unsigned int upperCutoff,
      bool partial,
      unsigned int* cropBounds,
      unsigned int* inputDim,
      bool useGPU,
      DelayCalc delayCalculationMethod,
      Apodization apod,
      unsigned int apodizationArraySize,
      BeamformingAlgorithm algorithm,
      bool useBP,
      float BPHighPass,
      float BPLowPass
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

    /** \brief Sets how many voxels should be cut off from the top of the image before beamforming, to potentially avoid artifacts.
    */
    unsigned int m_UpperCutoff;

    /** \brief Sets whether only the slices selected by mitk::BeamformingSettings::CropBounds should be beamformed.
    */
    bool m_Partial;

    /** \brief Sets the first and last slice to be beamformed.
    */
    const unsigned int* m_CropBounds;

    /** \brief Sets the dimensions of the inputImage.
    */
    const unsigned int* m_InputDim;

    /** \brief Decides whether GPU computing should be used
    */
    bool m_UseGPU;

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

    /** \brief Sets whether after beamforming a bandpass should be automatically applied
    */
    bool m_UseBP;

    /** \brief Sets the position at which lower frequencies are completely cut off in Hz.
    */
    float m_BPHighPass;

    /** \brief Sets the position at which higher frequencies are completely cut off in Hz.
    */
    float m_BPLowPass;
  };
}
#endif //MITK_BEAMFORMING_SETTINGS
