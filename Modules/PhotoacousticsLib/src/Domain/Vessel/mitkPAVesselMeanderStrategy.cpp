/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
