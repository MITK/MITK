/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkPAFluenceYOffsetPair.h"

mitk::pa::FluenceYOffsetPair::FluenceYOffsetPair(mitk::pa::Volume::Pointer fluenceValue, double yOffset)
{
  m_FluenceValue = fluenceValue;
  m_YOffsetInCentimeters = yOffset;
}

mitk::pa::FluenceYOffsetPair::~FluenceYOffsetPair()
{
  m_FluenceValue = nullptr;
}

mitk::pa::FluenceYOffsetPair::FluenceYOffsetPair(const mitk::pa::FluenceYOffsetPair::Pointer other)
{
  m_FluenceValue = other->GetFluenceValue();
  m_YOffsetInCentimeters = other->GetYOffsetInCentimeters();
}

mitk::pa::Volume::Pointer mitk::pa::FluenceYOffsetPair::GetFluenceValue() const
{
  return m_FluenceValue;
}

double mitk::pa::FluenceYOffsetPair::GetYOffsetInCentimeters() const
{
  return m_YOffsetInCentimeters;
}
