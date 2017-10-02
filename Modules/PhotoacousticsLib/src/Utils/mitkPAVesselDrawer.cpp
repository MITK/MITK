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

#include "mitkPAVesselDrawer.h"
#include "mitkPAVesselProperties.h"

mitk::pa::VesselDrawer::VesselDrawer()
{
}

mitk::pa::VesselDrawer::~VesselDrawer()
{
}

void mitk::pa::VesselDrawer::DrawVesselInVolume(Vector::Pointer fromPosition,
  VesselProperties::Pointer properties,
  InSilicoTissueVolume::Pointer volume)
{
  Vector::Pointer stepSize = properties->GetDirectionVector();

  fromPosition->SetElement(0, fromPosition->GetElement(0) + stepSize->GetElement(0));
  fromPosition->SetElement(1, fromPosition->GetElement(1) + stepSize->GetElement(1));
  fromPosition->SetElement(2, fromPosition->GetElement(2) + stepSize->GetElement(2));

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
}
