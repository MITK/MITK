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

void mitk::pa::VesselDrawer::FillInVolumeValues(mitk::pa::InSilicoTissueVolume::Pointer volume,
  mitk::pa::VesselProperties::Pointer properties,
  double vesselFraction,
  int x, int y, int z)
{
  if (vesselFraction <= 1 && vesselFraction >= 0)
  {
    double backgroundFraction = 1.0 - vesselFraction;

    double absorption = backgroundFraction *
      volume->GetAbsorptionVolume()->GetData(
        x, y, z)
      + vesselFraction * properties->GetAbsorptionCoefficient();
    double scattering = backgroundFraction *
      volume->GetScatteringVolume()->GetData(
        x, y, z)
      + vesselFraction * properties->GetScatteringCoefficient();
    double anisotropy = backgroundFraction *
      volume->GetAnisotropyVolume()->GetData(
        x, y, z)
      + vesselFraction * properties->GetAnisotopyCoefficient();

    volume->SetVolumeValues(x, y, z,
      absorption,
      scattering,
      anisotropy);

    if (vesselFraction >= 0.5)
    {
      m_VolumeState->SetData(VolumeState::PARTIAL, x, y, z);
    }
  }
}

void mitk::pa::VesselDrawer::InitializeVolumeState(InSilicoTissueVolume::Pointer volume)
{
  Volume::Pointer referenceVolume = volume->GetAbsorptionVolume();
  unsigned int xDim = referenceVolume->GetXDim();
  unsigned int yDim = referenceVolume->GetYDim();
  unsigned int zDim = referenceVolume->GetZDim();
  double* data = new double[xDim*yDim*zDim];
  for (unsigned int idx = 0, length = xDim*yDim*zDim; idx < length; ++idx)
  {
    data[idx] = VolumeState::UNTOUCHED;
  }

  m_VolumeState = Volume::New(data, xDim, yDim, zDim);
}

void mitk::pa::VesselDrawer::DrawVesselInVolume(Vector::Pointer fromPosition,
  VesselProperties::Pointer properties,
  InSilicoTissueVolume::Pointer volume)
{
  if (m_VolumeState.IsNull())
  {
    InitializeVolumeState(volume);
  }

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
  double ceiledRadius = ceil(radius) + 3;
  double radiusSquared = radius*radius;
  double PI = 3.141;
  double totalPoints = 1000;

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

        if (vectorLengthDiff < -2 ||
          sqrt(xPos*xPos + yPos*yPos + zPos*zPos) > 1)
        {
          continue;
        }

        if (vectorLengthDiff >= 1)
        {
          volume->SetVolumeValues(x, y, z,
            properties->GetAbsorptionCoefficient(),
            properties->GetScatteringCoefficient(),
            properties->GetAnisotopyCoefficient(),
            mitk::pa::InSilicoTissueVolume::SegmentationType::VESSEL);

          m_VolumeState->SetData(VolumeState::FULL, x, y, z);
        }
        else
        {
          if (properties->GetDoPartialVolume() &&
            abs(m_VolumeState->GetData(x, y, z) - VolumeState::UNTOUCHED) < mitk::eps)
          {
            double pointsWithin = 0;

            double phi = atan2(y, x);
            double theta = atan2(sqrt(x*x + y*y), z);
            double signx = (phi < PI || phi >(3 * PI / 2)) ? -1 : 1;
            double signy = (phi < PI) ? -1 : 1;
            double signz = (theta < PI / 2) ? -1 : 1;

            for (double checkx = 0; (checkx - 1.0) < -mitk::eps; checkx += 0.1)
              for (double checky = 0; (checky - 1.0) < -mitk::eps; checky += 0.1)
                for (double checkz = 0; (checkz - 1.0) < -mitk::eps; checkz += 0.1)
                {
                  double checkxSign = checkx*signx;
                  double checkySign = checky*signy;
                  double checkzSign = checkz*signz;
                  if (((xDiff - checkxSign) * (xDiff - checkxSign)
                    + (yDiff - checkySign) * (yDiff - checkySign)
                    + (zDiff - checkzSign) * (zDiff - checkzSign)) < radiusSquared)
                  {
                    pointsWithin += 1.0;
                  }
                }

            double vesselFraction = (pointsWithin / totalPoints);

            FillInVolumeValues(volume, properties, vesselFraction, x, y, z);
          }
        }
      }
}

bool mitk::pa::Equal(const VesselDrawer::Pointer leftHandSide, const VesselDrawer::Pointer rightHandSide, double eps, bool verbose)
{
  MITK_INFO(verbose) << "=== mitk::pa::Vessel Equal ===";

  if (rightHandSide.IsNull() || leftHandSide.IsNull())
  {
    MITK_INFO(verbose) << "Cannot compare nullpointers";
    return false;
  }

  //if (!Equal(leftHandSide->GetVolumeState(), rightHandSide->GetVolumeState(), eps, verbose))
  //{
  //  MITK_INFO(verbose) << "Volume state not equal";
  //  return false;
  //}

  return true;
}
