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
}

mitk::pa::Vessel::~Vessel()
{
  m_VesselProperties = nullptr;
  m_VesselMeanderStrategy = nullptr;
}

void mitk::pa::Vessel::ExpandVessel(InSilicoTissueVolume::Pointer volume,
  CalculateNewVesselPositionCallback calculateNewPosition, double bendingFactor, std::mt19937* rng)
{
  Vector::Pointer oldPosition = m_VesselProperties->GetPositionVector()->Clone();
  (m_VesselMeanderStrategy->*calculateNewPosition)(m_VesselProperties->GetPositionVector(), m_VesselProperties->GetDirectionVector(), bendingFactor, rng);
  DrawVesselInVolume(oldPosition, volume);
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

void mitk::pa::Vessel::DrawVesselInVolume(Vector::Pointer fromPosition,
  InSilicoTissueVolume::Pointer volume)
{
  Vector::Pointer diffVector = Vector::New();
  Vector::Pointer toPosition = m_VesselProperties->GetPositionVector();
  diffVector->SetElement(0, fromPosition->GetElement(0) - toPosition->GetElement(0));
  diffVector->SetElement(1, fromPosition->GetElement(1) - toPosition->GetElement(1));
  diffVector->SetElement(2, fromPosition->GetElement(2) - toPosition->GetElement(2));

  //1/SCALING_FACTOR steps along the direction vector are taken and drawn into the image.
  Vector::Pointer stepSize = Vector::New();
  stepSize->SetValue(m_VesselProperties->GetDirectionVector());
  stepSize->Scale(SCALING_FACTOR);

  while (diffVector->GetNorm() >= SCALING_FACTOR)
  {
    m_WalkedDistance += stepSize->GetNorm();

    fromPosition->SetElement(0, fromPosition->GetElement(0) + stepSize->GetElement(0));
    fromPosition->SetElement(1, fromPosition->GetElement(1) + stepSize->GetElement(1));
    fromPosition->SetElement(2, fromPosition->GetElement(2) + stepSize->GetElement(2));

    int xPos = fromPosition->GetElement(0);
    int yPos = fromPosition->GetElement(1);
    int zPos = fromPosition->GetElement(2);

    if (!volume->IsInsideVolume(xPos, yPos, zPos))
    {
      m_VesselProperties->SetRadiusInVoxel(0);
      break;
    }

    double radius = m_VesselProperties->GetRadiusInVoxel();

    for (int x = xPos - radius; x <= xPos + radius; x++)
      for (int y = yPos - radius; y <= yPos + radius; y++)
        for (int z = zPos - radius; z <= zPos + radius; z++)
        {
          if (radius*radius >= (x - xPos)*(x - xPos) + (y - yPos)*(y - yPos) + (z - zPos)*(z - zPos))
          {
            volume->SetVolumeValues(x, y, z, m_VesselProperties->GetAbsorptionCoefficient(),
              m_VesselProperties->GetScatteringCoefficient(),
              m_VesselProperties->GetAnisotopyCoefficient(),
              mitk::pa::InSilicoTissueVolume::SegmentationType::VESSEL);
          }
        }

    diffVector->SetElement(0, fromPosition->GetElement(0) - toPosition->GetElement(0));
    diffVector->SetElement(1, fromPosition->GetElement(1) - toPosition->GetElement(1));
    diffVector->SetElement(2, fromPosition->GetElement(2) - toPosition->GetElement(2));
  }
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
