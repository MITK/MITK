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

#include "mitkPAVesselTree.h"
#include "thread"

mitk::pa::VesselTree::VesselTree(VesselProperties::Pointer initialProperties)
{
  m_CurrentSubvessels = new std::vector<Vessel::Pointer>();
  Vessel::Pointer tmpVessel = Vessel::New(initialProperties);
  m_CurrentSubvessels->push_back(tmpVessel);
}

mitk::pa::VesselTree::~VesselTree()
{
  m_CurrentSubvessels->clear();
  delete m_CurrentSubvessels;
  m_CurrentSubvessels = nullptr;
}

void mitk::pa::VesselTree::Step(mitk::pa::InSilicoTissueVolume::Pointer volume,
  Vessel::CalculateNewVesselPositionCallback calculateNewPosition, double bendingFactor, std::mt19937* rng)
{
  std::vector<Vessel::Pointer> newVessels;

  for (unsigned int vesselTreeIndex = 0; vesselTreeIndex < m_CurrentSubvessels->size(); vesselTreeIndex++)
  {
    Vessel::Pointer currentVessel = m_CurrentSubvessels->at(vesselTreeIndex);
    if (!currentVessel->IsFinished())
    {
      currentVessel->ExpandVessel(volume, calculateNewPosition, bendingFactor, rng);
      if (currentVessel->CanBifurcate())
      {
        newVessels.push_back(currentVessel->Bifurcate(rng));
      }
    }
  }

  for (unsigned int newVesselsIndex = 0; newVesselsIndex < newVessels.size(); newVesselsIndex++)
  {
    m_CurrentSubvessels->push_back(newVessels.at(newVesselsIndex));
  }
}

bool mitk::pa::VesselTree::IsFinished()
{
  for (unsigned int vesselTreeIndex = 0; vesselTreeIndex < m_CurrentSubvessels->size(); vesselTreeIndex++)
  {
    if (!m_CurrentSubvessels->at(vesselTreeIndex)->IsFinished())
      return false;
  }
  return true;
}

bool mitk::pa::Equal(const VesselTree::Pointer leftHandSide, const VesselTree::Pointer rightHandSide, double eps, bool verbose)
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

  if (leftHandSide->GetCurrentSubvessels()->size() != rightHandSide->GetCurrentSubvessels()->size())
  {
    MITK_INFO(verbose) << "Not same number of subvessels in the vesseltree.";
    return false;
  }

  for (unsigned int vesselIdx = 0, numVessels = leftHandSide->GetCurrentSubvessels()->size(); vesselIdx < numVessels; ++vesselIdx)
  {
    if (!Equal(leftHandSide->GetCurrentSubvessels()->at(vesselIdx), rightHandSide->GetCurrentSubvessels()->at(vesselIdx), eps, verbose))
    {
      MITK_INFO(verbose) << "At least one vessel was not equal or the vessel list were not in order.";
      return false;
    }
  }

  return true;
}
