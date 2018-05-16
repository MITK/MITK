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
#include "mitkPAComposedVolume.h"
#include "mitkIOUtil.h"
#include "mitkImageReadAccessor.h"
#include <string>

mitk::pa::ComposedVolume::ComposedVolume(InSilicoTissueVolume::Pointer groundTruthVolume)
{
  m_GroundTruthVolume = groundTruthVolume;
  m_FluenceComponents = 0;
  m_FluenceValues.clear();
}

mitk::pa::ComposedVolume::~ComposedVolume()
{
  m_FluenceValues.clear();
}

int mitk::pa::ComposedVolume::GetNumberOfFluenceComponents()
{
  return m_FluenceComponents;
}

int mitk::pa::ComposedVolume::GetYOffsetForFluenceComponentInPixels(int fluenceComponent)
{
  if (fluenceComponent >= m_FluenceComponents)
    mitkThrow() << "Index out of bounds exception: There were less fluence components in the composed volume than the number you tried to access.";

  return (int)(m_FluenceValues.at(fluenceComponent)->GetYOffsetInCentimeters() /
    m_GroundTruthVolume->GetTissueParameters()->GetVoxelSpacingInCentimeters());
}

double mitk::pa::ComposedVolume::GetFluenceValue(int fluenceComponent, int x, int y, int z)
{
  if (fluenceComponent >= m_FluenceComponents)
    mitkThrow() << "Index out of bounds exception: There were less fluence components in the composed volume than the number you tried to access.";

  return m_FluenceValues.at(fluenceComponent)->GetFluenceValue()->GetData(x, y + GetYOffsetForFluenceComponentInPixels(fluenceComponent), z);
}

void mitk::pa::ComposedVolume::AddSlice(mitk::pa::FluenceYOffsetPair::Pointer pairToAdd)
{
  m_FluenceValues.push_back(pairToAdd);
  m_FluenceComponents++;
}

void mitk::pa::ComposedVolume::Sort()
{
  std::sort(m_FluenceValues.begin(), m_FluenceValues.end(), [](const mitk::pa::FluenceYOffsetPair::Pointer& struct1,
    const mitk::pa::FluenceYOffsetPair::Pointer& struct2)
  {
    return (struct1->GetYOffsetInCentimeters() < struct2->GetYOffsetInCentimeters());
  });
}
