/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPAVesselDrawer.h"
#include "mitkPAVesselProperties.h"

mitk::pa::VesselDrawer::VesselDrawer()
{
}

mitk::pa::VesselDrawer::~VesselDrawer()
{
}

void mitk::pa::VesselDrawer::ExpandAndDrawVesselInVolume(
  VesselProperties::Pointer properties,
  InSilicoTissueVolume::Pointer volume)
{
  Vector::Pointer stepDirection = properties->GetDirectionVector();
  Vector::Pointer fromPosition = properties->GetPositionVector()->Clone();

  Vector::Pointer totalWalkingDistance = stepDirection->Clone();
  totalWalkingDistance->Scale(1.0 / volume->GetSpacing());

  while (totalWalkingDistance->GetNorm() >= 1)
  {
    double xPos = fromPosition->GetElement(0);
    double yPos = fromPosition->GetElement(1);
    double zPos = fromPosition->GetElement(2);

    if (!volume->IsInsideVolume(xPos, yPos, zPos))
    {
      properties->SetRadiusInVoxel(0);
      return;
    }

    double radius = properties->GetRadiusInVoxel();
    double ceiledRadius = ceil(radius);

    for (int x = xPos - ceiledRadius; x <= xPos + ceiledRadius; x += 1)
      for (int y = yPos - ceiledRadius; y <= yPos + ceiledRadius; y += 1)
        for (int z = zPos - ceiledRadius; z <= zPos + ceiledRadius; z += 1)
        {
          if (!volume->IsInsideVolume(x, y, z))
          {
            continue;
          }
          double xDiff = x - xPos;
          double yDiff = y - yPos;
          double zDiff = z - zPos;
          double vectorLengthDiff = radius - sqrt(xDiff*xDiff + yDiff*yDiff + zDiff*zDiff);

          if (vectorLengthDiff > 0)
          {
            volume->SetVolumeValues(x, y, z,
              properties->GetAbsorptionCoefficient(),
              properties->GetScatteringCoefficient(),
              properties->GetAnisotopyCoefficient(),
              mitk::pa::InSilicoTissueVolume::SegmentationType::VESSEL);
          }
        }

    totalWalkingDistance->Subtract(stepDirection);
    fromPosition->Add(stepDirection);
  }

  properties->SetPositionVector(fromPosition);
}
