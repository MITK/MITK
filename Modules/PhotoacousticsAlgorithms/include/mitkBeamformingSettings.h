/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

    /** \brief Available geometries for Probes:
    * - Linear
    * - Concave
    */
    enum ProbeGeometry { Linear, Concave};

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
    itkGetConstMacro(ApodizationFunction, const float*);
    itkGetConstMacro(Apod, Apodization);
    itkGetConstMacro(ApodizationArraySize, int);
    itkGetConstMacro(Algorithm, BeamformingAlgorithm);
    itkGetConstMacro(ReconstructionDepth, float);
    itkGetConstMacro(Geometry, ProbeGeometry);
    itkGetConstMacro(ProbeRadius, float);
    itkGetConstMacro(ElementHeights, float*);
    itkGetConstMacro(ElementPositions, float*);
    itkGetConstMacro(HorizontalExtent, float);

    /** \brief function for mitk::PhotoacousticOCLBeamformingFilter to check whether buffers need to be updated
    * this method only checks parameters relevant for the openCL implementation
    */
    static bool SettingsChangedOpenCL(const BeamformingSettings::Pointer lhs, const BeamformingSettings::Pointer rhs)
    {
      return !((std::abs(lhs->GetAngle() - rhs->GetAngle()) < 0.01f) && // 0.01 degree error margin
        (lhs->GetApod() == rhs->GetApod()) &&
        (lhs->GetGeometry() == rhs->GetGeometry()) &&
        (std::abs(lhs->GetProbeRadius() - rhs->GetProbeRadius()) < 0.001f) &&
        (lhs->GetIsPhotoacousticImage() == rhs->GetIsPhotoacousticImage()) &&
        (std::abs(lhs->GetPitchInMeters() - rhs->GetPitchInMeters()) < 0.000001f) && // 0.0001 mm error margin
        (lhs->GetReconstructionLines() == rhs->GetReconstructionLines()) &&
        (lhs->GetSamplesPerLine() == rhs->GetSamplesPerLine()) &&
        (lhs->GetReconstructionDepth() == rhs->GetReconstructionDepth()) &&
        (std::abs(lhs->GetSpeedOfSound() - rhs->GetSpeedOfSound()) < 0.01f) &&
        (std::abs(lhs->GetTimeSpacing() - rhs->GetTimeSpacing()) < 0.00000000001f) && //0.01 ns error margin
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
      Apodization apod,
      unsigned int apodizationArraySize,
      BeamformingAlgorithm algorithm,
      ProbeGeometry geometry,
      float probeRadius)
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
        apod,
        apodizationArraySize,
        algorithm,
        geometry,
        probeRadius);
      smartPtr->UnRegister();
      return smartPtr;
    }

    unsigned short* GetMinMaxLines();

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
      Apodization apod,
      unsigned int apodizationArraySize,
      BeamformingAlgorithm algorithm,
      ProbeGeometry geometry,
      float probeRadius
    );

    ~BeamformingSettings() override;

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

    /** \brief Sets the used probe geometry
    */
    ProbeGeometry m_Geometry;

    /** \brief Sets the radius of the curved probe [m]
    */
    float m_ProbeRadius;

    /**
    */
    float *m_ElementHeights;

    /**
    */
    float *m_ElementPositions;

    /**
    */
    float m_HorizontalExtent;

    /**
    */
    unsigned short* m_MinMaxLines;
  };
}
#endif //MITK_BEAMFORMING_SETTINGS
