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

void mitk::pa::VesselMeanderStrategy::CalculateNewDirectionVectorInStraightLine(Vector::Pointer direction, double /*bendingFactor*/, std::mt19937* rng)
{
  if (direction->GetNorm() <= mitk::eps)
  {
    direction->Randomize(rng);
  }
}

void mitk::pa::VesselMeanderStrategy::CalculateNewRandomlyDivergingDirectionVector(Vector::Pointer direction, double bendingFactor, std::mt19937* rng)
{
  if (direction->GetNorm() <= mitk::eps)
  {
    direction->Randomize(rng);
  }

  direction->RandomizeByPercentage(RANDOMIZATION_PERCENTAGE, bendingFactor, rng);

  direction->Normalize();
}
