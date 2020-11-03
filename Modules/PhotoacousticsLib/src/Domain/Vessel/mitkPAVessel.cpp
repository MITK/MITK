/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPAVessel.h"
#include <chrono>
#include <thread>

mitk::pa::Vessel::Vessel(VesselProperties::Pointer initialProperties) :
  m_RangeDistribution(itk::Math::pi / 16, itk::Math::pi / 8),
  m_SignDistribution(-1, 1)
{
  m_Finished = false;
  //Copy this so it may be reused for other vessels.
  m_VesselProperties = VesselProperties::New(initialProperties);
  m_RadiusRangeDistribution = std::uniform_real_distribution<>(NEW_RADIUS_MINIMUM_RELATIVE_SIZE,
    NEW_RADIUS_MAXIMUM_RELATIVE_SIZE);
  m_VesselMeanderStrategy = VesselMeanderStrategy::New();
  m_WalkedDistance = 0;
  m_VesselDrawer = VesselDrawer::New();
}

mitk::pa::Vessel::~Vessel()
{
  m_VesselProperties = nullptr;
  m_VesselMeanderStrategy = nullptr;
}

void mitk::pa::Vessel::ExpandVessel(InSilicoTissueVolume::Pointer volume,
  CalculateNewVesselPositionCallback calculateNewPosition, double bendingFactor, std::mt19937* rng)
{
  m_VesselDrawer->ExpandAndDrawVesselInVolume(m_VesselProperties, volume);
  (m_VesselMeanderStrategy->*calculateNewPosition)(m_VesselProperties->GetDirectionVector(), bendingFactor, rng);
  m_WalkedDistance += (m_VesselProperties->GetDirectionVector()->GetNorm() / volume->GetSpacing());
}

bool mitk::pa::Vessel::CanBifurcate()
{
  return m_VesselProperties->GetBifurcationFrequency() < m_WalkedDistance;
}

int mitk::pa::Vessel::GetSign(std::mt19937 *rng)
{
  if (m_SignDistribution(*rng) < 0)
    return -1;

  return 1;
}

mitk::pa::Vessel::Pointer mitk::pa::Vessel::Bifurcate(std::mt19937* rng)
{
  VesselProperties::Pointer vesselParams = VesselProperties::New(m_VesselProperties);
  double thetaChange = m_RangeDistribution(*rng) * GetSign(rng);
  double phiChange = m_RangeDistribution(*rng) * GetSign(rng);

  vesselParams->GetDirectionVector()->Rotate(thetaChange, phiChange);
  m_VesselProperties->GetDirectionVector()->Rotate(-thetaChange, -phiChange);

  double newRadius = m_RadiusRangeDistribution(*rng)*m_VesselProperties->GetRadiusInVoxel();
  vesselParams->SetRadiusInVoxel(newRadius);
  m_VesselProperties->SetRadiusInVoxel(
    sqrt(m_VesselProperties->GetRadiusInVoxel()*m_VesselProperties->GetRadiusInVoxel() - newRadius*newRadius));

  m_WalkedDistance = 0;

  return Vessel::New(vesselParams);
}

bool mitk::pa::Vessel::IsFinished()
{
  return m_VesselProperties->GetRadiusInVoxel() < MINIMUM_VESSEL_RADIUS;
}

bool mitk::pa::Equal(const Vessel::Pointer leftHandSide, const Vessel::Pointer rightHandSide, double eps, bool verbose)
{
  MITK_INFO(verbose) << "=== mitk::pa::Vessel Equal ===";

  if (rightHandSide.IsNull() || leftHandSide.IsNull())
  {
    MITK_INFO(verbose) << "Cannot compare nullpointers";
    return false;
  }

  if (leftHandSide->IsFinished() != rightHandSide->IsFinished())
  {
    MITK_INFO(verbose) << "Not same finished state.";
    return false;
  }

  if (leftHandSide->CanBifurcate() != rightHandSide->CanBifurcate())
  {
    MITK_INFO(verbose) << "Not same bifurcation state.";
    return false;
  }

  if (!Equal(leftHandSide->GetVesselProperties(), rightHandSide->GetVesselProperties(), eps, verbose))
  {
    MITK_INFO(verbose) << "Vesselproperties not equal";
    return false;
  }

  return true;
}
