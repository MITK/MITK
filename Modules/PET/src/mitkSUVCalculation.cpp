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

#include <mitkSUVCalculation.h>

#include <cmath>

double mitk::computeSUVScaleFactor(double injectedActivity, double scaleNumerator, double decayTime, double halfLife)
{
  const double decayedDose = injectedActivity * std::pow(2.0, -decayTime / halfLife);
  return scaleNumerator / decayedDose;
}

double mitk::computeSUVbwScaleFactor(double injectedActivity, double bodyweight, double decayTime, double halfLife)
{
  return computeSUVScaleFactor(injectedActivity, bodyweight * 1000.0, decayTime, halfLife);
}

double mitk::computeSUVbw(double value, double injectedActivity, double bodyweight, double decayTime, double halfLife)
{
  return value * computeSUVbwScaleFactor(injectedActivity, bodyweight, decayTime, halfLife);
}
