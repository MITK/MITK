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

#include "mitkGeometryData.h"
#include "mitkBaseProcess.h"
#include "mitkTimeGeometry.h"

mitk::GeometryData::GeometryData()
{
}

mitk::GeometryData::~GeometryData()
{
}

void mitk::GeometryData::UpdateOutputInformation()
{
  Superclass::UpdateOutputInformation();
}

void mitk::GeometryData::SetRequestedRegionToLargestPossibleRegion()
{
}

bool mitk::GeometryData::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  if (GetGeometry() != nullptr)
    return true;

  return false;
}

bool mitk::GeometryData::VerifyRequestedRegion()
{
  if (GetGeometry() == nullptr)
    return false;

  return true;
}

void mitk::GeometryData::SetRequestedRegion(const itk::DataObject *)
{
}

void mitk::GeometryData::CopyInformation(const itk::DataObject *)
{
}

bool mitk::Equal(const mitk::GeometryData &leftHandSide,
                 const mitk::GeometryData &rightHandSide,
                 mitk::ScalarType eps,
                 bool verbose)
{
  unsigned int timeStepsLeft = leftHandSide.GetTimeGeometry()->CountTimeSteps();
  unsigned int timeStepsRight = rightHandSide.GetTimeGeometry()->CountTimeSteps();

  if (timeStepsLeft != timeStepsRight)
  {
    if (verbose)
    {
      MITK_INFO << "[( GeometryData::GetTimeGeometry )] number of time steps are not equal.";
      MITK_INFO << "rightHandSide is " << timeStepsRight << " : leftHandSide is " << timeStepsLeft;
    }
    return false;
  }

  bool allEqual = true;

  for (unsigned int t = 0; t < timeStepsLeft; ++t)
  {
    BaseGeometry *geomLeft = leftHandSide.GetGeometry(t);
    BaseGeometry *geomRight = rightHandSide.GetGeometry(t);
    allEqual &= mitk::Equal(*geomLeft, *geomRight, eps, verbose);
  }

  return allEqual; // valid with initial true for timestep count == 0
}
