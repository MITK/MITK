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