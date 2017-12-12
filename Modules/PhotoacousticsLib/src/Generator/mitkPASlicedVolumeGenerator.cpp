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

#include "mitkPASlicedVolumeGenerator.h"
#include <mitkException.h>

mitk::pa::SlicedVolumeGenerator::SlicedVolumeGenerator(int centralYSlice, bool precorrect,
  Volume::Pointer precorrectionVolume, bool inverse)
{
  m_CentralYSlice = centralYSlice;
  m_Precorrect = precorrect;
  m_Inverse = inverse;
  m_PrecorrectionVolume = nullptr;

  if (m_Precorrect)
  {
    m_PrecorrectionVolume = precorrectionVolume;
  }
}

mitk::pa::SlicedVolumeGenerator::~SlicedVolumeGenerator()
{
  m_CentralYSlice = -1;
  m_Precorrect = false;
  m_PrecorrectionVolume = nullptr;
}

mitk::pa::Volume::Pointer mitk::pa::SlicedVolumeGenerator::GetSlicedFluenceImageFromComposedVolume(
  ComposedVolume::Pointer composedVolume)
{
  int fluenceComponents = composedVolume->GetNumberOfFluenceComponents();

  int xDim = composedVolume->GetGroundTruthVolume()->GetAbsorptionVolume()->GetXDim();
  int zDim = composedVolume->GetGroundTruthVolume()->GetAbsorptionVolume()->GetZDim();

  auto* imageArray = new double[xDim*zDim*fluenceComponents];

  for (int fluenceComponentIdx = 0; fluenceComponentIdx < fluenceComponents; fluenceComponentIdx++)
    for (int z = 0; z < zDim; z++)
      for (int x = 0; x < xDim; x++)
      {
        int index = z * xDim * fluenceComponents + x * fluenceComponents + fluenceComponentIdx;
        imageArray[index] = composedVolume->GetFluenceValue(fluenceComponentIdx, x, m_CentralYSlice, z);

        if (m_Precorrect)
        {
          imageArray[index] = imageArray[index] / m_PrecorrectionVolume->GetData(x, m_CentralYSlice, z);
        }

        if (m_Inverse)
        {
          if (std::abs(imageArray[index] - 0) >= mitk::eps)
            imageArray[index] = 1 / imageArray[index];
          else
            imageArray[index] = INFINITY;
        }
      }

  return mitk::pa::Volume::New(imageArray, xDim, fluenceComponents, zDim);
}

mitk::pa::Volume::Pointer mitk::pa::SlicedVolumeGenerator::GetSlicedSignalImageFromComposedVolume(
  ComposedVolume::Pointer composedVolume)
{
  int fluenceComponents = composedVolume->GetNumberOfFluenceComponents();

  int xDim = composedVolume->GetGroundTruthVolume()->GetAbsorptionVolume()->GetXDim();
  int zDim = composedVolume->GetGroundTruthVolume()->GetAbsorptionVolume()->GetZDim();

  auto* imageArray = new double[xDim*zDim*fluenceComponents];

  for (int fluenceComponentIdx = 0; fluenceComponentIdx < fluenceComponents; fluenceComponentIdx++)
    for (int z = 0; z < zDim; z++)
      for (int x = 0; x < xDim; x++)
      {
        int y = m_CentralYSlice + composedVolume->GetYOffsetForFluenceComponentInPixels(fluenceComponentIdx);
        imageArray[z * xDim * fluenceComponents + x * fluenceComponents + fluenceComponentIdx] =
          composedVolume->GetFluenceValue(fluenceComponentIdx, x, m_CentralYSlice, z)
          * composedVolume->GetGroundTruthVolume()->GetAbsorptionVolume()->GetData(x, y, z);
      }

  return mitk::pa::Volume::New(imageArray, xDim, fluenceComponents, zDim);
}

mitk::pa::Volume::Pointer mitk::pa::SlicedVolumeGenerator::GetSlicedGroundTruthImageFromComposedVolume(
  ComposedVolume::Pointer composedVolume)
{
  int fluenceComponents = composedVolume->GetNumberOfFluenceComponents();

  int xDim = composedVolume->GetGroundTruthVolume()->GetAbsorptionVolume()->GetXDim();
  int zDim = composedVolume->GetGroundTruthVolume()->GetAbsorptionVolume()->GetZDim();

  auto* imageArray = new double[xDim*zDim*fluenceComponents];

  for (int fluenceComponentIdx = 0; fluenceComponentIdx < fluenceComponents; fluenceComponentIdx++)
    for (int z = 0; z < zDim; z++)
      for (int x = 0; x < xDim; x++)
      {
        int y = m_CentralYSlice + composedVolume->GetYOffsetForFluenceComponentInPixels(fluenceComponentIdx);
        imageArray[z * xDim * fluenceComponents + x * fluenceComponents + fluenceComponentIdx] =
          composedVolume->GetGroundTruthVolume()->GetAbsorptionVolume()->GetData(x, y, z);
      }

  return mitk::pa::Volume::New(imageArray, xDim, fluenceComponents, zDim);
}
