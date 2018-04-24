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

mitk::BeamformingSettings::BeamformingSettings(std::string xmlFile)
{
  MITK_ERROR << "Not implemented yet.";
  mitkThrow() << "Not implemented yet.";
}

mitk::BeamformingSettings::BeamformingSettings(float pitchInMeters,
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
) :
  m_PitchInMeters(pitchInMeters),
  m_SpeedOfSound(speedOfSound),
  m_TimeSpacing(timeSpacing),
  m_Angle(angle),
  m_IsPhotoacousticImage(isPhotoacousticImage),
  m_TransducerElements(transducerElements),
  m_UpperCutoff(upperCutoff),
  m_Partial(partial),
  m_CropBounds(cropBounds),
  m_UseGPU(useGPU),
  m_DelayCalculationMethod(delayCalculationMethod),
  m_Apod(apod),
  m_ApodizationArraySize(apodizationArraySize),
  m_Algorithm(algorithm),
  m_UseBP(useBP),
  m_BPHighPass(BPHighPass),
  m_BPLowPass(BPLowPass)
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

  if (m_CropBounds == nullptr)
    m_CropBounds = new unsigned int[2]{ 0, 0 };

  m_InputDim = new unsigned int[2]{ inputDim[0], inputDim[1] };

  m_SamplesPerLine = m_InputDim[1];
  m_ReconstructionLines = m_InputDim[0];
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

  if (m_CropBounds != nullptr)
  {
    MITK_INFO << "Deleting crop bounds...";
    delete[] m_CropBounds;
    MITK_INFO << "Deleting crop bounds...[Done]";
  }

  if (m_InputDim != nullptr)
  {
    MITK_INFO << "Deleting input dim...";
    delete[] m_InputDim;
    MITK_INFO << "Deleting input dim...[Done]";
  }

  MITK_INFO << "Destructing beamforming settings...[Done]";
}
