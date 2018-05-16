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

#include "mitkPAVesselMeanderStrategy.h"

mitk::pa::VesselMeanderStrategy::VesselMeanderStrategy()
{
}

mitk::pa::VesselMeanderStrategy::~VesselMeanderStrategy()
{
}

void mitk::pa::VesselMeanderStrategy::CalculateNewPositionInStraightLine(
  Vector::Pointer position, Vector::Pointer direction, double /*bendingFactor*/, std::mt19937* rng)
{
  if (direction->GetNorm() <= mitk::eps)
  {
    direction->Randomize(rng);
  }

  position->SetElement(0, position->GetElement(0) + direction->GetElement(0));
  position->SetElement(1, position->GetElement(1) + direction->GetElement(1));
  position->SetElement(2, position->GetElement(2) + direction->GetElement(2));
}

void mitk::pa::VesselMeanderStrategy::CalculateRandomlyDivergingPosition(
  Vector::Pointer position, Vector::Pointer direction, double bendingFactor, std::mt19937* rng)
{
  if (direction->GetNorm() <= mitk::eps)
  {
    direction->Randomize(rng);
  }

  direction->RandomizeByPercentage(RANDOMIZATION_PERCENTAGE, bendingFactor, rng);

  position->SetElement(0, position->GetElement(0) + direction->GetElement(0));
  position->SetElement(1, position->GetElement(1) + direction->GetElement(1));
  position->SetElement(2, position->GetElement(2) + direction->GetElement(2));
}
