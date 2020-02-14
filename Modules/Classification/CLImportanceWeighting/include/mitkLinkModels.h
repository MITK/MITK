/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLinkModels_h
#define mitkLinkModels_h

#include <MitkCLImportanceWeightingExports.h>

namespace mitk
{
  class MITKCLIMPORTANCEWEIGHTING_EXPORT AbstractLinkingFunction
  {
  public:
    virtual double Link(double mu) = 0;
    virtual double DLink(double mu) = 0;
    virtual double InverseLink(double eta) = 0;
  };

  class MITKCLIMPORTANCEWEIGHTING_EXPORT LogItLinking : AbstractLinkingFunction
  {
  public:
    LogItLinking();
    double Link(double mu) override;
    double DLink(double mu) override;
    double InverseLink(double eta) override;
  private:
    double lowerBound;
    double upperBound;
  };
}

#endif
