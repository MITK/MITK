/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  Apodization apod,
  unsigned int apodizationArraySize,
  BeamformingAlgorithm algorithm,
  ProbeGeometry geometry,
  float probeRadius
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
  m_Apod(apod),
  m_ApodizationArraySize(apodizationArraySize),
  m_Algorithm(algorithm),
  m_Geometry(geometry),
  m_ProbeRadius(probeRadius),
  m_MinMaxLines(nullptr)
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

  m_ElementHeights = new float[m_TransducerElements];
  m_ElementPositions = new float[m_TransducerElements];

  if (m_Geometry == ProbeGeometry::Concave)
  {
    float openingAngle = (m_TransducerElements * m_PitchInMeters) / (probeRadius * 2 * itk::Math::pi) * 2 * itk::Math::pi;
    m_HorizontalExtent = std::sin(openingAngle / 2.f) * probeRadius * 2.f;

    float elementAngle = 0;

    for (unsigned int i = 0; i < m_TransducerElements; ++i)
    {
      elementAngle = ((i- m_TransducerElements /2.f) * m_PitchInMeters) / (probeRadius * 2 * itk::Math::pi) * 2 * itk::Math::pi;
      m_ElementHeights[i] = probeRadius - std::cos(elementAngle) * probeRadius;
      m_ElementPositions[i] = m_HorizontalExtent/2.f + std::sin(elementAngle) * probeRadius;
    }
  }
  else
  {
    m_HorizontalExtent = m_PitchInMeters * m_TransducerElements;
    for (unsigned int i = 0; i < m_TransducerElements; ++i)
    {
      m_ElementHeights[i] = 0;
      m_ElementPositions[i] = i * m_PitchInMeters;
    }
  }
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
  if (m_ElementHeights != nullptr || m_ElementPositions != nullptr)
  {
    MITK_INFO << "Deleting element geometry...";
    if (m_ElementHeights != nullptr)
      delete[] m_ElementHeights;

    if (m_ElementPositions != nullptr)
      delete[] m_ElementPositions;
    MITK_INFO << "Destructing beamforming settings...[Done]";
  }
  if (m_MinMaxLines)
    delete[] m_MinMaxLines;
}

unsigned short* mitk::BeamformingSettings::GetMinMaxLines()
{
  if (!m_MinMaxLines)
    m_MinMaxLines = mitk::BeamformingUtils::MinMaxLines(this);
  return m_MinMaxLines;
}
