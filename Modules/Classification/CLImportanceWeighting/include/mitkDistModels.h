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