/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDistModels_h
#define mitkDistModels_h

#include <MitkCLImportanceWeightingExports.h>

namespace mitk
{
  class MITKCLIMPORTANCEWEIGHTING_EXPORT AbstractDistribution
  {
  public:
    virtual double SqrtVariance(double mu) = 0;
    virtual double Devariation(double mu, double y) = 0;
    virtual double Init(double y) = 0;
  };

  class MITKCLIMPORTANCEWEIGHTING_EXPORT DistSimpleBinominal : AbstractDistribution
  {
  public:
    double SqrtVariance(double mu) override;
    double Devariation(double mu, double y) override;
    double Init(double y) override;
  };
}

#endif
