/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkLinkModels.h>
#include <cmath>
#include <limits>

mitk::LogItLinking::LogItLinking()
{
  lowerBound = std::log( std::numeric_limits<double>::epsilon());
  upperBound = -lowerBound;
}

double mitk::LogItLinking::Link(double mu)
{
  return std::log(mu / (1 - mu));
}

double mitk::LogItLinking::DLink(double mu)
{
  return 1 / (mu * (1 - mu));
}

double mitk::LogItLinking::InverseLink(double eta)
{
  double result;
  if (eta < lowerBound)
  {
    result = 1 / (1 + std::exp(-lowerBound));
  }
  else if (eta > upperBound)
  {
    result = 1 / (1 + std::exp(-upperBound));
  }
  else
  {
    result = (1 / (1 + std::exp(-eta)));
  }
  if (result <0 ) result = 0;
  if (result > 1) result = 1;

  return result;
}
