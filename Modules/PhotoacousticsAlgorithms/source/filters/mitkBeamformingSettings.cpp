/*===================================================================
mitkBeamformingSettings
The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkBeamformingSettings.h"
#include "mitkBeamformingUtils.h"
#include "itkMutexLock.h"

mitk::BeamformingSettings::BeamformingSettings(float pitchInMeters,
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
) :
  m_PitchInMeters(pitchInMeters),
  m_SpeedOfSound(speedOfSound),
  m_TimeSpacing(timeSpacing),
  m_Angle(angle),
  m_IsPhotoacousticImage(isPhotoacousticImage),
  m_SamplesPerLine(samplesPerLine),
  m_ReconstructionLines(reconstructionLines),
  m_ReconstructionDepth(reconstructionDepth),
  m_UseGPU(useGPU),
  m_GPUBatchSize(GPUBatchSize),
  m_DelayCalculationMethod(delayCalculationMethod),
  m_Apod(apod),
  m_ApodizationArraySize(apodizationArraySize),
  m_Algorithm(algorithm)
{
  if (inputDim == nullptr)
  {
    MITK_ERROR << "No input dimension given.";
    mitkThrow() << "No input dimension given.";
  }

  switch (GetApod())
  {
  case BeamformingSettings::Apodization::Hann:
    m_ApodizationFunction = mitk::BeamformingUtils::VonHannFunction(GetApodizationArraySize());
    break;
  case BeamformingSettings::Apodization::Hamm:
    m_ApodizationFunction = mitk::BeamformingUtils::HammFunction(GetApodizationArraySize());
    break;
  case BeamformingSettings::Apodization::Box:
  default:
    m_ApodizationFunction = mitk::BeamformingUtils::BoxFunction(GetApodizationArraySize());
    break;
  }

  m_InputDim = new unsigned int[3]{ inputDim[0], inputDim[1], inputDim[2] };

  m_TransducerElements = m_InputDim[0];
}

mitk::BeamformingSettings::~BeamformingSettings()
{
  MITK_INFO << "Destructing beamforming settings...";
  //Free memory
  if (m_ApodizationFunction != nullptr)
  {
    MITK_INFO << "Deleting apodization function...";
    delete[] m_ApodizationFunction;
    MITK_INFO << "Deleting apodization function...[Done]";
  }

  if (m_InputDim != nullptr)
  {
    MITK_INFO << "Deleting input dim...";
    delete[] m_InputDim;
    MITK_INFO << "Deleting input dim...[Done]";
  }

  MITK_INFO << "Destructing beamforming settings...[Done]";
}
