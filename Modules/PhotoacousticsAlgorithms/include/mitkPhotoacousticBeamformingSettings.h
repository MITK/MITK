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

#include <itkObject.h>
#include <itkMacro.h>

namespace mitk {
  /*!
  * \brief Class holding the configuration data for the beamforming filters mitk::BeamformingFilter and mitk::PhotoacousticOCLBeamformingFilter
  *
  * A detailed description can be seen below. All parameters should be set manually for successfull beamforming.
  */

  class BeamformingSettings : public itk::Object
  {
  public:
    mitkClassMacroItkParent(BeamformingSettings, itk::Object)

      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

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

    itkGetMacro(Pitch, float)
      itkSetMacro(Pitch, float)
      itkGetMacro(SpeedOfSound, float)
      itkSetMacro(SpeedOfSound, float)
      itkGetMacro(RecordTime, float)
      itkSetMacro(RecordTime, float)
      itkGetMacro(TimeSpacing, float)
      itkSetMacro(TimeSpacing, float)
      itkGetMacro(Angle, float)
      itkSetMacro(Angle, float)
      itkGetMacro(IsPhotoacousticImage, bool)
      itkSetMacro(IsPhotoacousticImage, bool)
      itkGetMacro(TransducerElements, unsigned short)
      itkSetMacro(TransducerElements, unsigned short)
      itkGetMacro(SamplesPerLine, unsigned int)
      itkSetMacro(SamplesPerLine, unsigned int)
      itkGetMacro(ReconstructionLines, unsigned int)
      itkSetMacro(ReconstructionLines, unsigned int)
      itkGetMacro(UpperCutoff, unsigned int)
      itkSetMacro(UpperCutoff, unsigned int)
      itkGetMacro(Partial, bool)
      itkSetMacro(Partial, bool)
      itkGetMacro(CropBounds, unsigned int*)
      void SetCropBounds(unsigned int* cropBounds)
    {
      if (m_CropBounds != nullptr)
      {
        delete[] m_CropBounds;
      }

      m_CropBounds = cropBounds;
    }
    itkGetMacro(InputDim, unsigned int*)
      void SetInputDim(unsigned int* inputDim)
    {
      if (m_InputDim != nullptr)
      {
        delete[] m_InputDim;
      }

      m_InputDim = inputDim;
    }
    itkGetMacro(UseGPU, bool)
      itkSetMacro(UseGPU, bool)
      itkGetMacro(DelayCalculationMethod, DelayCalc)
      itkSetMacro(DelayCalculationMethod, DelayCalc)
      itkGetMacro(ApodizationFunction, float*)
      void SetApodizationFunction(float* function)
    {
      if (m_ApodizationFunction != nullptr)
      {
        delete[] m_ApodizationFunction;
      }

      m_ApodizationFunction = function;
    }
    itkGetMacro(Apod, Apodization)
      itkSetMacro(Apod, Apodization)
      itkGetMacro(ApodizationArraySize, int)
      itkSetMacro(ApodizationArraySize, int)
      itkGetMacro(Algorithm, BeamformingAlgorithm)
      itkSetMacro(Algorithm, BeamformingAlgorithm)
      itkGetMacro(UseBP, bool)
      itkSetMacro(UseBP, bool)
      itkGetMacro(BPHighPass, float)
      itkSetMacro(BPHighPass, float)
      itkGetMacro(BPLowPass, float)
      itkSetMacro(BPLowPass, float)

      /** \brief function for mitk::PhotoacousticOCLBeamformingFilter to check whether buffers need to be updated
      * this method only checks parameters relevant for the openCL implementation
      */
      static bool SettingsChangedOpenCL(const BeamformingSettings::Pointer lhs, const BeamformingSettings::Pointer rhs)
    {
      return !((abs(lhs->GetAngle() - rhs->GetAngle()) < 0.01f) && // 0.01 degree error margin
        (lhs->GetApod() == rhs->GetApod()) &&
        (lhs->GetDelayCalculationMethod() == rhs->GetDelayCalculationMethod()) &&
        (lhs->GetIsPhotoacousticImage() == rhs->GetIsPhotoacousticImage()) &&
        (abs(lhs->GetPitch() - rhs->GetPitch()) < 0.000001f) && // 0.0001 mm error margin
        (lhs->GetReconstructionLines() == rhs->GetReconstructionLines()) &&
        (abs(lhs->GetRecordTime() - rhs->GetRecordTime()) < 0.00000001f) && // 10 ns error margin
        (lhs->GetSamplesPerLine() == rhs->GetSamplesPerLine()) &&
        (abs(lhs->GetSpeedOfSound() - rhs->GetSpeedOfSound()) < 0.01f) &&
        (abs(lhs->GetTimeSpacing() - rhs->GetTimeSpacing()) < 0.00000000001f) && //0.01 ns error margin
        (lhs->GetTransducerElements() == rhs->GetTransducerElements()));
    }

  protected:
    BeamformingSettings()
    {
    }
    ~BeamformingSettings()
    {
    }

    /** \brief Pitch of the used transducer in [m].
    */
    float m_Pitch = 0.0003;

    /** \brief Speed of sound in the used medium in [m/s].
    */
    float m_SpeedOfSound = 1540;

    /** \brief This parameter is not neccessary to be set, as it's never used.
    */
    float m_RecordTime = 0.00006; // [s]

    /** \brief The time spacing of the input image
    */
    float m_TimeSpacing = 0.0000000000001; // [s]

    /** \brief The angle of the transducer elements
    */
    float m_Angle = -1;

    /** \brief Flag whether processed image is a photoacoustic image or an ultrasound image
    */
    bool m_IsPhotoacousticImage = true;

    /** \brief How many transducer elements the used transducer had.
    */
    unsigned short m_TransducerElements = 128;

    /** \brief How many vertical samples should be used in the final image.
    */
    unsigned int m_SamplesPerLine = 2048;

    /** \brief How many lines should be reconstructed in the final image.
    */
    unsigned int m_ReconstructionLines = 128;

    /** \brief Sets how many voxels should be cut off from the top of the image before beamforming, to potentially avoid artifacts.
    */
    unsigned int m_UpperCutoff = 0;

    /** \brief Sets whether only the slices selected by mitk::BeamformingSettings::CropBounds should be beamformed.
    */
    bool m_Partial = false;
    /** \brief Sets the first and last slice to be beamformed.
    */
    unsigned int* m_CropBounds = new unsigned int[2]{ 0, 0 };

    /** \brief Sets the dimensions of the inputImage.
    */
    unsigned int* m_InputDim = new unsigned int[3]{ 1, 1, 1 };

    /** \brief Decides whether GPU computing should be used
    */
    bool m_UseGPU = false;

    /** \brief Sets how the delays for beamforming should be calculated.
    */
    DelayCalc m_DelayCalculationMethod = QuadApprox;

    float* m_ApodizationFunction;

    /** \brief Sets the used apodization function.
    */
    Apodization m_Apod = Hann;

    /** \brief Sets the resolution of the apodization array (must be greater than 0).
    */
    int m_ApodizationArraySize = 128;

    /** \brief Sets the used beamforming algorithm.
    */
    BeamformingAlgorithm m_Algorithm = DAS;

    /** \brief Sets whether after beamforming a bandpass should be automatically applied
    */
    bool m_UseBP = false;

    /** \brief Sets the position at which lower frequencies are completely cut off in Hz.
    */
    float m_BPHighPass = 50;

    /** \brief Sets the position at which higher frequencies are completely cut off in Hz.
    */
    float m_BPLowPass = 50;
  };
}
#endif
