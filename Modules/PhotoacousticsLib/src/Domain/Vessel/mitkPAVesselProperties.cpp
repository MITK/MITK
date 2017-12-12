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

#include "mitkPAVesselProperties.h"

mitk::pa::VesselProperties::VesselProperties()
{
  m_PositionVector = Vector::New();
  m_DirectionVector = Vector::New();
  m_RadiusInVoxel = 0;
  m_AbsorptionCoefficient = 0;
  m_ScatteringCoefficient = 0;
  m_AnisotopyCoefficient = 0;
  m_BifurcationFrequency = 0;
}

mitk::pa::VesselProperties::VesselProperties(Self::Pointer other)
{
  m_PositionVector = other->GetPositionVector()->Clone();
  m_DirectionVector = other->GetDirectionVector()->Clone();
  m_RadiusInVoxel = other->GetRadiusInVoxel();
  m_AbsorptionCoefficient = other->GetAbsorptionCoefficient();
  m_ScatteringCoefficient = other->GetScatteringCoefficient();
  m_AnisotopyCoefficient = other->GetAnisotopyCoefficient();
  m_BifurcationFrequency = other->GetBifurcationFrequency();
}

mitk::pa::VesselProperties::~VesselProperties()
{
  m_PositionVector = nullptr;
  m_DirectionVector = nullptr;
}

bool mitk::pa::Equal(const VesselProperties::Pointer leftHandSide, const VesselProperties::Pointer rightHandSide,
  double eps, bool verbose)
{
  MITK_INFO(verbose) << "=== mitk::pa::VesselProperties Equal ===";

  if (rightHandSide.IsNull() || leftHandSide.IsNull())
  {
    MITK_INFO(verbose) << "Cannot compare nullpointers";
    return false;
  }

  if (leftHandSide->GetAbsorptionCoefficient() - rightHandSide->GetAbsorptionCoefficient() > eps)
  {
    MITK_INFO(verbose) << "Not the same AbsorptionCoefficient.";
    return false;
  }

  if (leftHandSide->GetAnisotopyCoefficient() - rightHandSide->GetAnisotopyCoefficient() > eps)
  {
    MITK_INFO(verbose) << "Not the same AnisotropyCoefficient.";
    return false;
  }

  if (leftHandSide->GetBifurcationFrequency() - rightHandSide->GetBifurcationFrequency() > eps)
  {
    MITK_INFO(verbose) << "Not the same BifurcationFrequency.";
    return false;
  }

  if (leftHandSide->GetRadiusInVoxel() - rightHandSide->GetRadiusInVoxel() > eps)
  {
    MITK_INFO(verbose) << "Not the same RadiusInVoxel.";
    return false;
  }

  if (leftHandSide->GetScatteringCoefficient() - rightHandSide->GetScatteringCoefficient() > eps)
  {
    MITK_INFO(verbose) << "Not the same ScatteringCoefficient.";
    return false;
  }

  if (!Equal(leftHandSide->GetPositionVector(), rightHandSide->GetPositionVector(), eps, verbose))
  {
    MITK_INFO(verbose) << "PositionVector not equal";
    return false;
  }

  if (!Equal(leftHandSide->GetDirectionVector(), rightHandSide->GetDirectionVector(), eps, verbose))
  {
    MITK_INFO(verbose) << "PositionVector not equal";
    return false;
  }

  return true;
}
